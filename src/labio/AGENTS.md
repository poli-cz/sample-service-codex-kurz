# AGENTS.md for src/labio

Adds to the root AGENTS.md; where the two differ, this file wins inside `src/labio/`.

## Legacy firmware parser (`legacy/`)

- `legacy_result_parser.cpp` decodes the lines the analyzer firmware sends. Units in
  the field still send firmware v1 lines (`R;pos;sample;assay;signal`); both formats
  must keep working.
- Do not change the layout of `LEGACY_RESULT` or the `LR_*` constants in
  `legacy_result_parser.h`. The old daemon dumped the struct to disk and tools still
  read those files.
- Before refactoring anything here, add characterization tests in
  `tests/labio/result_parser_test.cpp` that pin the current behavior, including the
  odd parts (silent truncation of long fields, `OK` flag ignored, trimming of spaces).
  Run them green on the old code first, then refactor.
- Refactor in small steps behind the existing facade `lab::io::parse_reading`; its
  signature and the `ParseOutcome` variant are used by `labsched` and must not change.
- `legacy/.clang-tidy` and the `COMPILE_OPTIONS` in `src/labio/CMakeLists.txt` list the
  accepted exceptions for this file. When a refactoring removes the reason for an
  exception, remove the exception in the same change.

## Reports and config

- CSV output follows RFC 4180. Consumers are spreadsheet tools, keep the column order.
- Config errors throw `ConfigError` with a message that names the field.
