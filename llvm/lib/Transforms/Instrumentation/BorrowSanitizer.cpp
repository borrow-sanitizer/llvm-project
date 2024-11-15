//===- BorrowSanitizer.cpp - Instrumentation for overflow defense
//------------===//

#include "llvm/Transforms/Instrumentation/BorrowSanitizer.h"
#include "llvm/Transforms/Instrumentation.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/Local.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"

#include "llvm/Analysis/GlobalsModRef.h"
#include "llvm/Analysis/TargetLibraryInfo.h"

#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Module.h"


#include "llvm/Support/CommandLine.h"

using namespace llvm;

#define DEBUG_TYPE "bsan"

// Constants:
const char kBsanModuleCtorName[] = "bsan.module_ctor";
const char kBsannModuleDtorName[] = "bsan.module_dtor";
const char kAsanInitName[] = "__bsan_init";

// Command-line flags:
namespace {

struct BorrowSanitizer {

  BorrowSanitizer(Module &M)  {
    C = &(M.getContext());
    DL = &M.getDataLayout();
    LongSize = M.getDataLayout().getPointerSizeInBits();
    IntptrTy = Type::getIntNTy(*C, LongSize);
    PtrTy = PointerType::getUnqual(*C);
    Int32Ty = Type::getInt32Ty(*C);
    TargetTriple = Triple(M.getTargetTriple());
  }
  bool instrumentFunction(Function &F, const TargetLibraryInfo *TLI);

private: 
  LLVMContext *C;
  const DataLayout *DL;
  int LongSize;
  Triple TargetTriple;
  Type *IntptrTy;
  Type *Int32Ty;
  PointerType *PtrTy;
};

class ModuleBorrowSanitizer {
public:
  ModuleBorrowSanitizer(Module &M) {
    C = &(M.getContext());
    int LongSize = M.getDataLayout().getPointerSizeInBits();
    IntptrTy = Type::getIntNTy(*C, LongSize);
    PtrTy = PointerType::getUnqual(*C);
    TargetTriple = Triple(M.getTargetTriple());
  }

  bool instrumentModule(Module &);
private:
  Type *IntptrTy;
  PointerType *PtrTy;
  LLVMContext *C;
  Triple TargetTriple;
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
    Modified |= FunctionSanitizer.instrumentFunction(F, &TLI);
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


bool ModuleBorrowSanitizer::instrumentModule(Module &M) {
  return true;
}

bool BorrowSanitizer::instrumentFunction(Function &F,
                                          const TargetLibraryInfo *TLI) {
  if (F.empty())
    return false;

  if (F.getLinkage() == GlobalValue::AvailableExternallyLinkage) return false;
  if (F.getName().starts_with("__bsan_")) return false;
  if (F.isPresplitCoroutine())
    return false;

  bool FunctionModified = false;
  
  return FunctionModified;
}