# Build for the AddressSanitizer runtime support library.

set(BSAN_SOURCES
  bsan.cpp
  )

set(BSAN_CXX_SOURCES
  
  )

SET(BSAN_HEADERS
  bsan.h
  borrowtracker.h
)

include_directories(..)

set(BSAN_CFLAGS ${SANITIZER_COMMON_CFLAGS})

set(BSAN_COMMON_DEFINITIONS "")

append_rtti_flag(OFF BSAN_CFLAGS)

# Too many existing bugs, needs cleanup.
append_list_if(COMPILER_RT_HAS_WNO_FORMAT -Wno-format BSAN_CFLAGS)

set(BSAN_DYNAMIC_LINK_FLAGS ${SANITIZER_COMMON_LINK_FLAGS})

set(BSAN_DYNAMIC_DEFINITIONS
  ${BSAN_COMMON_DEFINITIONS} BSAN_DYNAMIC=1)

set(BSAN_DYNAMIC_CFLAGS ${BSAN_CFLAGS})
append_list_if(COMPILER_RT_HAS_FTLS_MODEL_INITIAL_EXEC
  -ftls-model=initial-exec BSAN_DYNAMIC_CFLAGS)

set(BSAN_DYNAMIC_LIBS
  ${COMPILER_RT_UNWINDER_LINK_LIBS}
  ${SANITIZER_CXX_ABI_LIBRARIES}
  ${SANITIZER_COMMON_LINK_LIBS}
  borrowtracker)

if(COMPILER_RT_DEFAULT_TARGET_ONLY)
  set(BSAN_SUPPORTED_ARCH ${CMAKE_HOST_SYSTEM_PROCESSOR})
endif()

append_list_if(COMPILER_RT_HAS_LIBDL dl BSAN_DYNAMIC_LIBS)
append_list_if(COMPILER_RT_HAS_LIBRT rt BSAN_DYNAMIC_LIBS)
append_list_if(COMPILER_RT_HAS_LIBM m BSAN_DYNAMIC_LIBS)
append_list_if(COMPILER_RT_HAS_LIBPTHREAD pthread BSAN_DYNAMIC_LIBS)
append_list_if(COMPILER_RT_HAS_LIBLOG log BSAN_DYNAMIC_LIBS)

# Compile bsan sources into an object library.

add_compiler_rt_object_libraries(RTBsan_dynamic
  OS ${SANITIZER_COMMON_SUPPORTED_OS}
  ARCHS ${BSAN_SUPPORTED_ARCH}
  SOURCES ${BSAN_SOURCES} ${BSAN_CXX_SOURCES}
  ADDITIONAL_HEADERS ${BSAN_HEADERS}
  CFLAGS ${BSAN_DYNAMIC_CFLAGS}
  DEFS ${BSAN_DYNAMIC_DEFINITIONS})

if(NOT APPLE)
  add_compiler_rt_object_libraries(RTBsan
    ARCHS ${BSAN_SUPPORTED_ARCH}
    SOURCES ${BSAN_SOURCES}
    ADDITIONAL_HEADERS ${BSAN_HEADERS}
    CFLAGS ${BSAN_CFLAGS}
    DEFS ${BSAN_COMMON_DEFINITIONS})

  #add_compiler_rt_object_libraries(RTBsan_cxx
  #  ARCHS ${BSAN_SUPPORTED_ARCH}
  #  SOURCES ${BSAN_CXX_SOURCES}
  #  ADDITIONAL_HEADERS ${BSAN_HEADERS}
  #  CFLAGS ${BSAN_CFLAGS}
  #  DEFS ${BSAN_COMMON_DEFINITIONS})

  file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp "")
  add_compiler_rt_object_libraries(RTBsan_dynamic_version_script_dummy
    ARCHS ${BSAN_SUPPORTED_ARCH}
    SOURCES ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp
    CFLAGS ${BSAN_DYNAMIC_CFLAGS}
    DEFS ${BSAN_DYNAMIC_DEFINITIONS})
endif()

# Build BSan runtimes shipped with Clang.
add_compiler_rt_component(bsan)

if(APPLE)
  add_weak_symbols("bsan" WEAK_SYMBOL_LINK_FLAGS)
  add_weak_symbols("sanitizer_common" WEAK_SYMBOL_LINK_FLAGS)

  add_compiler_rt_runtime(clang_rt.bsan
    SHARED
    OS ${SANITIZER_COMMON_SUPPORTED_OS}
    ARCHS ${BSAN_SUPPORTED_ARCH}
    OBJECT_LIBS RTBsan_dynamic
                RTInterception
                RTSanitizerCommon
                RTSanitizerCommonLibc
                RTSanitizerCommonCoverage
                RTSanitizerCommonSymbolizer
    CFLAGS ${BSAN_DYNAMIC_CFLAGS}
    LINK_FLAGS ${WEAK_SYMBOL_LINK_FLAGS}
    DEFS ${BSAN_DYNAMIC_DEFINITIONS}
    LINK_LIBS ${BSAN_DYNAMIC_LIBS}
    PARENT_TARGET bsan)

else()
  # Build separate libraries for each target.

  set(BSAN_COMMON_RUNTIME_OBJECT_LIBS
    RTInterception
    RTSanitizerCommon
    RTSanitizerCommonLibc
    RTSanitizerCommonCoverage
    RTSanitizerCommonSymbolizer
    RTSanitizerCommonSymbolizerInternal
  )

  add_compiler_rt_runtime(clang_rt.bsan
    STATIC
    ARCHS ${BSAN_SUPPORTED_ARCH}
    OBJECT_LIBS RTBsan
                ${BSAN_COMMON_RUNTIME_OBJECT_LIBS}
    CFLAGS ${BSAN_CFLAGS}
    DEFS ${BSAN_COMMON_DEFINITIONS}
    PARENT_TARGET bsan)

#  add_compiler_rt_runtime(clang_rt.bsan_cxx
#    STATIC
#    ARCHS ${BSAN_SUPPORTED_ARCH}
#    OBJECT_LIBS RTBsan_cxx
#    CFLAGS ${BSAN_CFLAGS}
#    DEFS ${BSAN_COMMON_DEFINITIONS}
#    PARENT_TARGET bsan)

  foreach(arch ${BSAN_SUPPORTED_ARCH})
    if (COMPILER_RT_HAS_VERSION_SCRIPT)
      add_sanitizer_rt_version_list(clang_rt.bsan-dynamic-${arch}
                                    LIBS clang_rt.bsan-${arch} # clang_rt.bsan_cxx-${arch}
                                    EXTRA bsan.syms.extra)
      set(VERSION_SCRIPT_FLAG
           -Wl,--version-script,${CMAKE_CURRENT_BINARY_DIR}/clang_rt.bsan-dynamic-${arch}.vers)
      # The Solaris 11.4 linker supports a subset of GNU ld version scripts,
      # but requires a special option to enable it.
      if (COMPILER_RT_HAS_GNU_VERSION_SCRIPT_COMPAT)
          list(APPEND VERSION_SCRIPT_FLAG -Wl,-z,gnu-version-script-compat)
      endif()
      set_property(SOURCE
        ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp
        APPEND PROPERTY
        OBJECT_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/clang_rt.bsan-dynamic-${arch}.vers)
    else()
      set(VERSION_SCRIPT_FLAG)
    endif()

    set(BSAN_DYNAMIC_WEAK_INTERCEPTION)

    add_compiler_rt_runtime(clang_rt.bsan
      SHARED
      ARCHS ${arch}
      OBJECT_LIBS ${BSAN_COMMON_RUNTIME_OBJECT_LIBS}
              RTBsan_dynamic
              # The only purpose of RTBsan_dynamic_version_script_dummy is to
              # carry a dependency of the shared runtime on the version script.
              # Replacing it with a straightforward
              # add_dependencies(clang_rt.bsan-dynamic-${arch} clang_rt.bsan-dynamic-${arch}-version-list)
              # generates an order-only dependency in ninja.
              RTBsan_dynamic_version_script_dummy
              ${BSAN_DYNAMIC_WEAK_INTERCEPTION}
      CFLAGS ${BSAN_DYNAMIC_CFLAGS}
      LINK_FLAGS ${BSAN_DYNAMIC_LINK_FLAGS}
                ${VERSION_SCRIPT_FLAG}
      LINK_LIBS ${BSAN_DYNAMIC_LIBS}
      DEFS ${BSAN_DYNAMIC_DEFINITIONS}
      PARENT_TARGET bsan)

    if (SANITIZER_USE_SYMBOLS)
      # add_sanitizer_rt_symbols(clang_rt.bsan_cxx
      #  ARCHS ${arch})
      # add_dependencies(bsan clang_rt.bsan_cxx-${arch}-symbols)
      add_sanitizer_rt_symbols(clang_rt.bsan
        ARCHS ${arch}
        EXTRA bsan.syms.extra)
      add_dependencies(bsan clang_rt.bsan-${arch}-symbols)
    endif()
  endforeach()
endif()

add_compiler_rt_resource_file(bsan_ignorelist bsan_ignorelist.txt bsan)

# add_subdirectory(scripts)

if(COMPILER_RT_INCLUDE_TESTS)
  add_subdirectory(tests)
endif()