# Kurz OpenAI Codex v C++ týmu

Materiály ke kurzu agentního vývoje s OpenAI Codexem (ICT Pro): slidy, zadání cvičení a vzorový
projekt, na kterém cvičení běží. Projekt `sample-service` je smyšlená laboratorní služba v C++20.
Není v něm žádný firemní kód ani skutečná data, smíte ho dát jakémukoli agentovi.

| co | kde |
|---|---|
| slidy obou dnů (PDF) | [`prezentace/`](prezentace/) |
| zadání cvičení C0 až C13 | [`cviceni/`](cviceni/README.md) |
| MCP server pro cvičení C7 (Python) | [`mcp-lab-server/`](mcp-lab-server/) |
| vzorový projekt `sample-service` | tento repozitář, popis níže (anglicky) |

## Začátek

```sh
git clone https://github.com/poli-cz/sample-service-codex-kurz sample-service
cd sample-service
python scripts/check_all.py debug
```

Poslední příkaz projekt nakonfiguruje, postaví a spustí všechny testy. Co je potřeba mít
nainstalované, popisuje cvičení [C0](cviceni/00-priprava/ZADANI.md). Každé cvičení začíná ve větvi,
kterou jmenuje jeho zadání (například `git switch cviceni/04-start`). Přehled větví je
v [`cviceni/README.md`](cviceni/README.md).

---

# sample-service

A small C++20 service that manages the sample queue of a (simulated) laboratory
analyzer: samples are submitted with a priority, loaded onto a rack, measured,
converted to concentrations with a calibration curve and flagged against the
reportable range. Results are written as JSON lines and optionally as CSV.

The instrument is simulated and deterministic, so every run with the same input
gives the same output.

## Layout

```
src/labcore/     domain model: samples, priority queue, calibration, statistics
src/labio/       analyzer line parser (with the legacy firmware parser), JSON, config, CSV reports
src/labsched/    simulated instrument, rack, scheduler
apps/sample-service/  executable and the command line protocol
tests/           GoogleTest suites, one per library, plus an end-to-end smoke test
scripts/         Python helpers (batch generator, result summary) and their tests
config/          default service configuration
examples/        command batches
docs/            architecture notes
```

Dependency direction: `apps -> labsched -> labio -> labcore`. `labcore` depends on
the standard library only.

## Requirements

- CMake 3.22 or newer and Ninja
- GCC 12+, Clang 16+ or MSVC 19.30+ (Visual Studio 2022 or newer) with C++20 support
- Third-party libraries {fmt} 9+, nlohmann_json 3.10+ and GoogleTest 1.11+: installed ones
  are found with `find_package()`, missing ones are built from `third_party/`
- Python 3.9+ (only for `scripts/` and their tests)
- clang-tidy 14+ (only for the `tidy` preset)

Verified toolchains: GCC 12.2 and Clang 16 on Debian 12 (see `Dockerfile`), and AppleClang 21
on macOS 26 with Homebrew (CMake 4.4, fmt 12.2, nlohmann_json 3.12, GoogleTest 1.18,
clang-tidy 23 from the `llvm` formula), and on Windows MSVC 19.51 with Visual Studio 2026 and
MSVC 19.44 with Visual Studio 2022 and CMake 3.24 (libraries from `third_party/`, clang-tidy from
Visual Studio). On macOS the `asan` preset runs without
LeakSanitizer, which Apple's toolchain does not support, and the `asan` test preset turns off
ASan's container-overflow check, which reports false positives when GoogleTest from Homebrew
(not instrumented) is mixed with instrumented code; the `coverage` preset is GCC only. With MSVC
the `asan` preset has AddressSanitizer only, MSVC has no UndefinedBehaviorSanitizer.

## Dependencies

CMake looks for each library with `find_package()` first. A library it does not find is built
from the copy in `third_party/` (versions and licenses in `third_party/README.md`), so the
project builds without any of the sources below and without network access. To use the
installed libraries, pick one of:

| source | how |
|---|---|
| system packages, Debian/Ubuntu | `apt install cmake ninja-build g++ libfmt-dev nlohmann-json3-dev libgtest-dev libgmock-dev clang-tidy python3` |
| system packages, macOS | `brew install cmake ninja fmt nlohmann-json googletest llvm` |
| Windows, Visual Studio | workload "Desktop development with C++" with C++ CMake tools, C++ AddressSanitizer and C++ Clang tools; run CMake from the Developer PowerShell. The libraries come from `third_party/`, or from vcpkg with the `vcpkg` preset |
| vcpkg (manifest mode) | clone vcpkg, set `VCPKG_ROOT`, use the `vcpkg` preset; versions are pinned by `builtin-baseline` in `vcpkg.json` |
| prebuilt prefix | configure with `-DCMAKE_PREFIX_PATH=/path/to/prefix` |

## Build and test in one command

```sh
python scripts/check_all.py debug    # configure + build + all tests
```

The script runs the three CMake steps of the preset, the same as by hand:

```sh
cmake --preset debug
cmake --build --preset debug
ctest --preset debug
```

| preset | what it is |
|---|---|
| `debug` | Debug build, all tests |
| `release` | Release build, all tests |
| `asan` | Debug with AddressSanitizer and UndefinedBehaviorSanitizer |
| `tidy` | Debug with clang-tidy on every first-party translation unit, warnings are errors |
| `coverage` | Debug with `--coverage` (GCC), for line coverage with gcov or gcovr |
| `vcpkg` | Debug with dependencies from vcpkg (needs `VCPKG_ROOT`) |

Every preset builds into `build/<preset>/`. `python scripts/check_all.py` without arguments
runs `debug`, `release`, `asan` and `tidy` one after another.

Faster feedback while working on one library:

```sh
cmake --build --preset debug --target labcore_tests
ctest --preset debug -R SampleQueue          # only tests whose name matches
ctest --preset debug -N                      # list tests without running them
```

## Running the service

```sh
build/debug/apps/sample-service/sample-service --config config/service.json \
    --batch examples/basic.batch
```

Without `--batch` it reads commands from standard input. One command per line:

| command | effect |
|---|---|
| `SUBMIT <id> <assay> [STAT\|URGENT\|ROUTINE]` | queue a sample, default ROUTINE; ids look like `S-004711` |
| `CANCEL <id>` | remove a sample from the queue or the rack |
| `LOAD` | fill free rack positions from the queue |
| `RUN` | load the rack if it is empty, measure everything on it, empty the rack |
| `STATUS` | queue counts and rack contents |
| `REPORT <file.csv>` | write all results so far as CSV |
| `SUMMARY <file.csv>` | write per-assay statistics as CSV |
| `QUIT` | stop |

Lines starting with `#` are comments. Each output line is a JSON object with an
`event` field (`submit`, `load`, `result`, `cycle_done`, `status`, `log`, `error`, ...).

A STAT sample submitted while the rack is loaded and full takes the place of the
lowest priority sample on the rack; that sample goes back to the head of the queue.

## Scripts

```sh
scripts/gen_batch.py --samples 40 --stat-share 0.1 --seed 3 > /tmp/load.batch
build/debug/apps/sample-service/sample-service --batch /tmp/load.batch | scripts/summarize_results.py
```

Line coverage per source file (GCC and gcov):

```sh
python scripts/check_all.py coverage
scripts/coverage_report.py
```

The unit tests of the scripts run as part of `ctest` (`scripts.unittest`).

## Static analysis

`.clang-tidy` in the repository root lists the enabled check groups and every
disabled check with the reason. `src/labio/legacy/.clang-tidy` adds exceptions for
the legacy firmware parser only. Compiler warnings are errors (`-Werror`) except for
the categories listed in `src/labio/CMakeLists.txt` for the same legacy file.

## Container

`Dockerfile` builds a Debian based toolchain image with everything above. It is a
convenience, not a requirement:

```sh
docker build -t sample-service-dev .
docker run --rm -it -v "$PWD":/work -u "$(id -u):$(id -g)" sample-service-dev \
    python3 scripts/check_all.py debug
```
