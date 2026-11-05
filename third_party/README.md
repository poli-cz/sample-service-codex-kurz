# third_party

Unmodified copies of the three dependencies, trimmed to what the build needs (no docs, tests
or samples). CMake uses them only for a library that `find_package()` does not find, so a
machine without vcpkg or system packages still builds the project without network access.

| library | version | source | license |
|---|---|---|---|
| {fmt} | 12.2.0 | https://github.com/fmtlib/fmt/releases/tag/12.2.0 | MIT, `fmt/LICENSE` |
| GoogleTest | 1.18.0 (googletest only, no googlemock) | https://github.com/google/googletest/releases/tag/v1.18.0 | BSD-3-Clause, `googletest/LICENSE` |
| nlohmann_json | 3.12.0 (single header) | https://github.com/nlohmann/json/releases/tag/v3.12.0 | MIT, `nlohmann_json/LICENSE.MIT` |

Do not edit these files. An update replaces a whole directory with a newer release.
