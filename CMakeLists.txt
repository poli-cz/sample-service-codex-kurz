cmake_minimum_required(VERSION 3.22...3.25)

project(sample_service
  VERSION 0.4.0
  DESCRIPTION "Sample queue and measurement service for a simulated lab analyzer"
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(SAMPLE_BUILD_TESTS "Build the GoogleTest suites" ON)
option(SAMPLE_ENABLE_SANITIZERS "Build with AddressSanitizer and UBSan" OFF)
option(SAMPLE_ENABLE_CLANG_TIDY "Run clang-tidy on every translation unit" OFF)
option(SAMPLE_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" ON)
option(SAMPLE_ENABLE_COVERAGE "Instrument for line coverage (GCC --coverage)" OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(CompilerWarnings)
include(Sanitizers)
include(ClangTidy)

# Third-party dependencies are found with find_package(), so any of these works
# without touching this file:
#   * system packages (Debian/Ubuntu, Homebrew),
#   * vcpkg manifest mode (preset "vcpkg", see vcpkg.json),
#   * a prebuilt prefix passed as -DCMAKE_PREFIX_PATH=/opt/labdeps.
# Whatever is not found is built from the copies in third_party/.
find_package(fmt 9 QUIET)
find_package(nlohmann_json 3.10 QUIET)
if(SAMPLE_BUILD_TESTS)
  find_package(GTest 1.11 QUIET)
endif()
add_subdirectory(third_party)

add_subdirectory(src/labcore)
add_subdirectory(src/labio)
add_subdirectory(src/labsched)
add_subdirectory(apps/sample-service)

if(SAMPLE_BUILD_TESTS)
  enable_testing()
  include(GoogleTest)
  add_subdirectory(tests)
endif()
