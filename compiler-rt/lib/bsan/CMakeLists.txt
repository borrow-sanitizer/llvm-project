include_directories(..)

set(BSAN_SOURCES
  bsan_interceptors.cpp
  bsan.cpp
  )

set(BSAN_PREINIT_SOURCES
  bsan_preinit.cpp
  )

SET(BSAN_HEADERS
  bsan.h
  bsan_interceptors.h
  bsanrt.h
)

set(BSAN_DEPS)

set(BSAN_CFLAGS
  ${COMPILER_RT_COMMON_CFLAGS}
  ${COMPILER_RT_CXX_CFLAGS}
  -DSANITIZER_COMMON_NO_REDEFINE_BUILTINS)
set(BSAN_LINK_FLAGS ${COMPILER_RT_COMMON_LINK_FLAGS})
set(BSAN_LINK_LIBS
  bsan_rt
  ${COMPILER_RT_UNWINDER_LINK_LIBS}
  ${COMPILER_RT_CXX_LINK_LIBS})
  
if(APPLE)
  add_compiler_rt_object_libraries(RTBsan
    OS ${SANITIZER_COMMON_SUPPORTED_OS}
    ARCHS ${BSAN_SUPPORTED_ARCH}
    SOURCES ${BSAN_SOURCES}
    ADDITIONAL_HEADERS ${BSAN_HEADERS}
    CFLAGS ${BSAN_CFLAGS}
    DEPS ${BSAN_DEPS})
else()
  add_compiler_rt_object_libraries(RTBsan
    ARCHS ${BSAN_SUPPORTED_ARCH}
    SOURCES ${BSAN_SOURCES}
    ADDITIONAL_HEADERS ${BSAN_HEADERS}
    CFLAGS ${BSAN_CFLAGS}
    DEPS ${BSAN_DEPS})
  add_compiler_rt_object_libraries(RTBsan_preinit
    ARCHS ${BSAN_SUPPORTED_ARCH}
    SOURCES ${BSAN_PREINIT_SOURCES}
    ADDITIONAL_HEADERS ${BSAN_HEADERS}
    CFLAGS ${BSAN_CFLAGS})
endif()

set(BSAN_COMMON_RUNTIME_OBJECT_LIBS
  RTInterception
  RTSanitizerCommon
  RTSanitizerCommonLibc
  RTSanitizerCommonCoverage
  RTSanitizerCommonSymbolizer)

append_list_if(COMPILER_RT_HAS_LIBDL dl BSAN_LINK_LIBS)
append_list_if(COMPILER_RT_HAS_LIBRT rt BSAN_LINK_LIBS)
append_list_if(COMPILER_RT_HAS_LIBM m BSAN_LINK_LIBS)
append_list_if(COMPILER_RT_HAS_LIBPTHREAD pthread BSAN_LINK_LIBS)
append_list_if(COMPILER_RT_HAS_LIBLOG log BSAN_LINK_LIBS)

add_compiler_rt_component(bsan)

if (APPLE)
  add_weak_symbols("sanitizer_common" WEAK_SYMBOL_LINK_FLAGS)
  add_weak_symbols("bsan" WEAK_SYMBOL_LINK_FLAGS)
  set(BSAN_LINK_FLAGS ${BSAN_LINK_FLAGS} ${WEAK_SYMBOL_LINK_FLAGS})

  add_compiler_rt_runtime(clang_rt.bsan
    SHARED
    OS ${SANITIZER_COMMON_SUPPORTED_OS}
    ARCHS ${BSAN_SUPPORTED_ARCH}
    OBJECT_LIBS RTBsan
                ${BSAN_COMMON_RUNTIME_OBJECT_LIBS}
    LINK_FLAGS ${BSAN_LINK_FLAGS}
    LINK_LIBS ${BSAN_LINK_LIBS}
    PARENT_TARGET bsan) 
else()
  add_compiler_rt_runtime(clang_rt.bsan
    STATIC
    ARCHS ${BSAN_SUPPORTED_ARCH}
    OBJECT_LIBS RTBsan
                RTBsan_preinit
                ${BSAN_COMMON_RUNTIME_OBJECT_LIBS}
    LINK_FLAGS ${BSAN_LINK_FLAGS}
    CFLAGS ${BSAN_CFLAGS}
    PARENT_TARGET bsan)
endif()