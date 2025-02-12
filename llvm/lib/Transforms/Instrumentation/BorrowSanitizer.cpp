//===- BorrowSanitizer.cpp - Instrumentation for BorrowSanitizer
//------------===//
#include "llvm/Transforms/Instrumentation/BorrowSanitizer.h"
#include "llvm/Transforms/Instrumentation.h"

#include "llvm/ADT/DepthFirstIterator.h"

#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/Local.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"

#include "llvm/Analysis/GlobalsModRef.h"
#include "llvm/Analysis/TargetLibraryInfo.h"

#include "llvm/IR/DataLayout.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/Module.h"

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/DebugCounter.h"

#include "llvm/Transforms/Utils/EscapeEnumerator.h"

using namespace llvm;

#define DEBUG_TYPE "bsan"

// Constants:
const char kBsanModuleCtorName[] = "bsan.module_ctor";
const char kBsanModuleDtorName[] = "bsan.module_dtor";
const char kBsanInitName[] = "__bsan_init";
const char kBsanFuncEntryName[] = "__bsan_func_entry";
const char kBsanFuncExitName[] = "__bsan_func_exit";
const char kBsanRetagName[] = "__bsan_retag";

// Command-line flags:
static cl::opt<bool>
    ClEnableKbsan("bsan-kernel",
                  cl::desc("Enable KernelBorrowSanitizer instrumentation"),
                  cl::Hidden, cl::init(false));

static cl::opt<bool> ClBsanAliasingModel(
    "bsan-model",
    cl::desc("Choose which of Rust's aliasing models to use (stack, tree)."),
    cl::Hidden, cl::init("tree"));

static cl::opt<bool>
    ClWithComdat("bsan-with-comdat",
                 cl::desc("Place BSan constructors in comdat sections"),
                 cl::Hidden, cl::init(true));

static cl::opt<bool> ClHandleCxxExceptions(
    "bsan-handle-cxx-exceptions", cl::init(true),
    cl::desc("Handle C++ exceptions (insert cleanup blocks for unwinding)"),
    cl::Hidden);

namespace {

struct BorrowSanitizer {
  BorrowSanitizer(Module &M) {
    C = &(M.getContext());
    DL = &M.getDataLayout();
    LongSize = M.getDataLayout().getPointerSizeInBits();
    Int8Ty = Type::getInt8Ty(*C);
    PtrTy = PointerType::getUnqual(*C);
    TargetTriple = Triple(M.getTargetTriple());
  }
  bool instrumentAlloca(Instruction &I);
  bool instrumentMemIntrinsic(Instruction &I);
  bool instrumentLoad(Instruction &I);
  bool instrumentStore(Instruction &I);
  bool instrumentRetag(Instruction &I);
  bool instrumentCall(Instruction &I);
  bool instrumentIntToPtr(Instruction &I);
  bool instrumentPtrToInt(Instruction &I);
  bool instrumentFunction(Function &F, const TargetLibraryInfo &TLI);

private:
  friend struct BorrowSanitizerVisitor;

  void initializeCallbacks(Module &M, const TargetLibraryInfo &TLI);

  LLVMContext *C;
  const DataLayout *DL;
  int LongSize;
  Triple TargetTriple;
  Type *Int8Ty;
  PointerType *PtrTy;

  FunctionCallee BsanRetagFunc;
  FunctionCallee BsanFuncEntry;
  FunctionCallee BsanFuncExit;
};

class ModuleBorrowSanitizer {
public:
  ModuleBorrowSanitizer(Module &M)
      : CompileKernel(ClEnableKbsan.getNumOccurrences() > 0 ? ClEnableKbsan
                                                            : CompileKernel),
        UseCtorComdat(ClWithComdat && !this->CompileKernel) {
    C = &(M.getContext());
    int LongSize = M.getDataLayout().getPointerSizeInBits();
    Int8Ty = Type::getInt8Ty(*C);
    IntptrTy = Type::getIntNTy(*C, LongSize);
    PtrTy = PointerType::getUnqual(*C);
    TargetTriple = Triple(M.getTargetTriple());
  }
  bool instrumentModule(Module &);

private:
  void initializeCallbacks(Module &);
  void instrumentGlobals(IRBuilder<> &IRB, Module &M, bool *CtorComdat);
  Instruction *CreateBsanModuleDtor(Module &M);
  bool CompileKernel;
  bool UseCtorComdat;
  LLVMContext *C;
  Type *Int8Ty;
  Type *IntptrTy;
  PointerType *PtrTy;
  Triple TargetTriple;
  Function *BsanCtorFunction = nullptr;
  Function *BsanDtorFunction = nullptr;
};

} // end anonymous namespace

PreservedAnalyses BorrowSanitizerPass::run(Module &M,
                                           ModuleAnalysisManager &MAM) {
  ModuleBorrowSanitizer ModuleSanitizer(M);
  bool Modified = false;
  auto &FAM = MAM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();

  for (Function &F : M) {
    BorrowSanitizer FunctionSanitizer(M);
    const TargetLibraryInfo &TLI = FAM.getResult<TargetLibraryAnalysis>(F);
    Modified |= FunctionSanitizer.instrumentFunction(F, TLI);
  }
  Modified |= ModuleSanitizer.instrumentModule(M);
  if (!Modified)
    return PreservedAnalyses::all();
  Modified |= ModuleSanitizer.instrumentModule(M);

  PreservedAnalyses PA = PreservedAnalyses::none();
  // GlobalsAA is considered stateless and does not get invalidated unless
  // explicitly invalidated; PreservedAnalyses::none() is not enough. Sanitizers
  // make changes that require GlobalsAA to be invalidated.
  PA.abandon<GlobalsAA>();
  return PA;
}

Instruction *ModuleBorrowSanitizer::CreateBsanModuleDtor(Module &M) {
  BsanDtorFunction = Function::createWithDefaultAttr(
      FunctionType::get(Type::getVoidTy(*C), false),
      GlobalValue::InternalLinkage, 0, kBsanModuleDtorName, &M);
  BsanDtorFunction->addFnAttr(Attribute::NoUnwind);
  // Ensure Dtor cannot be discarded, even if in a comdat.
  appendToUsed(M, {BsanDtorFunction});
  BasicBlock *BsanDtorBB = BasicBlock::Create(*C, "", BsanDtorFunction);
  return ReturnInst::Create(*C, BsanDtorBB);
}

bool ModuleBorrowSanitizer::instrumentModule(Module &M) {
  if (CompileKernel) {
    // The kernel always builds with its own runtime, and therefore does not
    // need the init and version check calls.
    BsanCtorFunction = createSanitizerCtor(M, kBsanModuleCtorName);
  } else {
    // TODO(ian): add version check.
    std::tie(BsanCtorFunction, std::ignore) =
        createSanitizerCtorAndInitFunctions(M, kBsanModuleCtorName,
                                            kBsanInitName, /*InitArgTypes=*/{},
                                            /*InitArgs=*/{}, "");
  }

  bool CtorComdat = true;

  IRBuilder<> IRB(BsanCtorFunction->getEntryBlock().getTerminator());
  instrumentGlobals(IRB, M, &CtorComdat);

  assert(BsanCtorFunction && BsanDtorFunction);
  uint64_t Priority = 1;

  // Put the constructor and destructor in comdat if both
  // (1) global instrumentation is not TU-specific
  // (2) target is ELF.
  if (UseCtorComdat && TargetTriple.isOSBinFormatELF() && CtorComdat) {
    BsanCtorFunction->setComdat(M.getOrInsertComdat(kBsanModuleCtorName));
    appendToGlobalCtors(M, BsanCtorFunction, Priority, BsanCtorFunction);

    BsanDtorFunction->setComdat(M.getOrInsertComdat(kBsanModuleDtorName));
    appendToGlobalDtors(M, BsanDtorFunction, Priority, BsanDtorFunction);

  } else {
    appendToGlobalCtors(M, BsanCtorFunction, Priority);
    appendToGlobalDtors(M, BsanDtorFunction, Priority);
  }

  return true;
}

void ModuleBorrowSanitizer::instrumentGlobals(IRBuilder<> &IRB, Module &M,
                                              bool *CtorComdat) {
  CreateBsanModuleDtor(M);
}

void ModuleBorrowSanitizer::initializeCallbacks(Module &M) {
  IRBuilder<> IRB(*C);
}

void BorrowSanitizer::initializeCallbacks(Module &M,
                                          const TargetLibraryInfo &TLI) {
  IRBuilder<> IRB(*C);
  BsanRetagFunc = M.getOrInsertFunction(kBsanRetagName, IRB.getVoidTy(), PtrTy,
                                        Int8Ty, Int8Ty);
  BsanFuncEntry = M.getOrInsertFunction(kBsanFuncEntryName, IRB.getVoidTy());
  BsanFuncExit = M.getOrInsertFunction(kBsanFuncExitName, IRB.getVoidTy());

}

bool BorrowSanitizer::instrumentAlloca(Instruction &I) {
  return false;
}

bool BorrowSanitizer::instrumentMemIntrinsic(Instruction &I) {
  return false;
}

bool BorrowSanitizer::instrumentLoad(Instruction &I) {
  return false;
}

bool BorrowSanitizer::instrumentStore(Instruction &I) {
  return false;
}

bool BorrowSanitizer::instrumentCall(Instruction &I) {
  return false;
}

bool BorrowSanitizer::instrumentIntToPtr(Instruction &I) {
  return false;
}

bool BorrowSanitizer::instrumentPtrToInt(Instruction &I) {
  return false;
}

bool BorrowSanitizer::instrumentRetag(Instruction &I) {
  CallInst* CIRetag = CallInst::Create(BsanRetagFunc, {I.getOperand(0), I.getOperand(1), I.getOperand(2)});
  ReplaceInstWithInst(&I, CIRetag);
  return true;
}

bool BorrowSanitizer::instrumentFunction(Function &F,
                                         const TargetLibraryInfo &TLI) {
  if (F.empty())
    return false;

  if (F.getLinkage() == GlobalValue::AvailableExternallyLinkage)
    return false;
  if (F.getName().starts_with("__bsan_"))
    return false;
  if (F.isPresplitCoroutine())
    return false;

  initializeCallbacks(*F.getParent(), TLI);

  SmallVector<Instruction *, 8> Loads;
  SmallVector<Instruction *, 8> Stores;
  SmallVector<Instruction *, 8> Retags;
  SmallVector<Instruction *, 8> Allocas;
  SmallVector<Instruction *, 8> MemIntrinsics;
  SmallVector<Instruction *, 8> Calls;

  SmallVector<Instruction *, 8> PtrToInts;
  SmallVector<Instruction *, 8> IntToPtrs;

  for (auto &BB : F) {
    for (auto &Inst : BB) {
      // Skip instructions inserted by another sanitizer
      if (Inst.hasMetadata(LLVMContext::MD_nosanitize))
        continue;

      unsigned Opc = Inst.getOpcode();
      switch (Opc) {
      // Memory instructions
      case Instruction::Load: {
        Loads.push_back(&Inst);
      } break;
      case Instruction::Store: {
        Stores.push_back(&Inst);
      } break;
      case Instruction::Alloca: {
        Allocas.push_back(&Inst);
      } break;
      case Instruction::Call: {
        if (IntrinsicInst *II = dyn_cast<IntrinsicInst>(&Inst)) {
          if (II->getIntrinsicID() == Intrinsic::bsan_retag) {
            Retags.push_back(&Inst);
          } else if (MemIntrinsic *MI = dyn_cast<MemIntrinsic>(II)) {
            MemIntrinsics.push_back(&Inst);
          }
        } else {
          Calls.push_back(&Inst);
        }
      } break;
      }
    }
  }

  InstrumentationIRBuilder IRB(F.getEntryBlock().getFirstNonPHI());
  IRB.CreateCall(BsanFuncEntry, {});

  
  EscapeEnumerator EE(F, "bsan_cleanup", ClHandleCxxExceptions);
  while (IRBuilder<> *AtExit = EE.Next()) {
    InstrumentationIRBuilder::ensureDebugInfo(*AtExit, F);
    AtExit->CreateCall(BsanFuncExit, {});
  }

  for (auto *I : Allocas) {
    instrumentAlloca(*I);
  }

  for (auto *I : MemIntrinsics) {
    instrumentMemIntrinsic(*I);
  }

  for (auto *I : Loads) {
    instrumentLoad(*I);
  }

  for (auto *I : Stores) {
    instrumentStore(*I);
  }

  for (auto *I : Retags) {
    instrumentRetag(*I);
  }

  for (auto *I : Calls) {
    instrumentCall(*I);
  }
  return true;
}