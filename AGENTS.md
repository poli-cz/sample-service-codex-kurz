# AGENTS.md

Instructions for coding agents working in this repository. Humans: see README.md.

## Project

`sample-service` is a C++20 service for the sample queue of a simulated lab analyzer.
Libraries: `src/labcore` (domain, no dependencies), `src/labio` (parsing, JSON, config,
CSV), `src/labsched` (instrument, rack, scheduler). Executable: `apps/sample-service`.
Dependency direction is `apps -> labsched -> labio -> labcore`; never add an include
that points the other way.

## Build and test

- Everything, one command: `python scripts/check_all.py debug`
- Before you say a change is done, also run: `python scripts/check_all.py tidy`
- Any change that touches memory ownership, containers or lifetimes (rack, queue,
  scheduler, parser): also `python scripts/check_all.py asan`
- Fast loop for one library: `cmake --build --preset debug --target labsched_tests`
  and `ctest --preset debug -R <TestSuiteName>`
- Do not create other build directories; presets build into `build/<preset>/`.

## Dependencies

- Third-party libraries: {fmt}, nlohmann_json, GoogleTest. `find_package()` takes the
  installed ones; whatever it does not find is built from the copies in `third_party/`.
  Do not edit `third_party/`, do not add `FetchContent`, do not add git submodules.
- A new dependency needs a human decision. Propose it in the plan, add it to
  `vcpkg.json`, `third_party/` and the README table only after approval.
- If the build fails on a dependency, stop and report which one; do not work around it.

## Conventions

- C++20, GCC 12+, Clang 16+ and MSVC 19.30+ (Visual Studio 2022). No compiler-specific extensions.
- Formatting: `.clang-format` (Google based, 100 columns).
- Names: types `PascalCase`, functions and variables `snake_case`, members end with `_`,
  constants `kPascalCase`. Namespaces `lab`, `lab::io`, `lab::sched`, `lab::stats`.
- Errors: exceptions for configuration and programming errors, `std::optional` or
  result types for expected outcomes (see `SubmitOutcome`, `ParseOutcome`).
- No new warnings. `-Werror` is on; do not add `-Wno-*` flags or `NOLINT` comments to
  make a build pass. If a clang-tidy check is wrong for the code, say so in the MR
  description instead.

## Tests

- GoogleTest, one test executable per library in `tests/<library>/`.
- New behavior starts with a failing test. Bug fixes start with a test that reproduces
  the bug.
- Test names describe behavior: `TEST(SampleQueue, ServesStatThenUrgentThenRoutine)`.
- The simulated instrument is deterministic; use it instead of mocks.
- Python scripts in `scripts/` have unittest tests in `scripts/tests/`, run by ctest.

## Do not touch without asking

- `CMakePresets.json` and `cmake/`: shared by the whole team.
- `.clang-tidy` files: exceptions may only be removed, never added.
- `config/service.json` calibrators: they model a real calibration.
- `src/labio/legacy/`: see `src/labio/AGENTS.md`.

## Done means

1. `python scripts/check_all.py debug tidy` passes (and `asan` when the rule above applies).
2. New or changed behavior has tests.
3. README.md is updated when a command, preset or dependency changes.
4. The merge request description has: what and why, how it was tested (commands),
   risks, and anything left open.
