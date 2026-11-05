# When enabled, clang-tidy runs as part of the compilation of first-party
# targets. Checks and documented exceptions are in .clang-tidy files.
if(SAMPLE_ENABLE_CLANG_TIDY)
  find_program(CLANG_TIDY_EXE NAMES clang-tidy clang-tidy-18 clang-tidy-17 clang-tidy-16 clang-tidy-15 clang-tidy-14
    # Homebrew keeps LLVM out of PATH (keg-only); Visual Studio installs it with the
    # "C++ Clang tools for Windows" component.
    HINTS /opt/homebrew/opt/llvm/bin /usr/local/opt/llvm/bin "$ENV{VCINSTALLDIR}/Tools/Llvm/x64/bin"
    REQUIRED)
  message(STATUS "clang-tidy: ${CLANG_TIDY_EXE}")
endif()

function(sample_enable_tidy target)
  if(SAMPLE_ENABLE_CLANG_TIDY)
    # The compile commands may come from GCC; clang-tidy must not fail on GCC-only warning flags.
    set(args "${CLANG_TIDY_EXE};--extra-arg=-Wno-unknown-warning-option")
    if(MSVC)
      # clang-tidy in cl mode drops /EHsc from the command line it gets from CMake and then
      # reports every throw as an error; pass it again.
      list(APPEND args "--extra-arg=/EHsc")
    endif()
    set_target_properties(${target} PROPERTIES CXX_CLANG_TIDY "${args}")
  endif()
endfunction()
