# Copies of the dependencies for machines that do not have them installed. Each one is
# built only when find_package() in the top-level CMakeLists.txt did not find it.
# Their headers are system headers, so first-party warnings and clang-tidy skip them.

if(NOT fmt_FOUND)
  message(STATUS "fmt: copy in third_party/")
  set(FMT_SYSTEM_HEADERS ON)
  add_subdirectory(fmt EXCLUDE_FROM_ALL)
endif()

if(NOT nlohmann_json_FOUND)
  message(STATUS "nlohmann_json: copy in third_party/")
  add_library(nlohmann_json INTERFACE)
  add_library(nlohmann_json::nlohmann_json ALIAS nlohmann_json)
  target_include_directories(nlohmann_json SYSTEM INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/nlohmann_json/include")
endif()

if(SAMPLE_BUILD_TESTS AND NOT GTest_FOUND)
  message(STATUS "GoogleTest: copy in third_party/")
  set(BUILD_GMOCK OFF)
  set(INSTALL_GTEST OFF)
  # Same C runtime as the rest of the build (/MD) with MSVC.
  set(gtest_force_shared_crt ON)
  add_subdirectory(googletest EXCLUDE_FROM_ALL)
  set_target_properties(gtest PROPERTIES
    INTERFACE_SYSTEM_INCLUDE_DIRECTORIES $<TARGET_PROPERTY:gtest,INTERFACE_INCLUDE_DIRECTORIES>)
endif()
