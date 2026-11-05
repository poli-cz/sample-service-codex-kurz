# Sanitizers and coverage instrumentation are enabled globally so that tests and
# third-party headers see the same instrumentation as the service.
if(SAMPLE_ENABLE_SANITIZERS)
  if(MSVC)
    # MSVC has AddressSanitizer but no UBSan, and ASan cannot be combined with the /RTC
    # run-time checks that CMake adds to Debug builds.
    add_compile_options(/fsanitize=address)
    add_link_options(/INCREMENTAL:NO)
    string(REGEX REPLACE "/RTC[1csu]*" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    # The vcpkg libraries are not instrumented; with the STL container annotations on,
    # mixing them with instrumented code fails to link (LNK2038 annotate_vector).
    add_compile_definitions(_DISABLE_VECTOR_ANNOTATION _DISABLE_STRING_ANNOTATION)
  else()
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer -fno-sanitize-recover=undefined)
    add_link_options(-fsanitize=address,undefined)
  endif()
  # GoogleTest from system packages is not instrumented. With libc++ (macOS) its inlined
  # container code then trips false "container-overflow" reports, so the asan test preset
  # sets detect_container_overflow=0. Test discovery must run under that environment too,
  # hence at test time instead of right after the build.
  set(CMAKE_GTEST_DISCOVER_TESTS_DISCOVERY_MODE PRE_TEST)
endif()

if(SAMPLE_ENABLE_COVERAGE)
  add_compile_options(--coverage -O0)
  add_link_options(--coverage)
endif()
