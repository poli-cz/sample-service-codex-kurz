# analyzer-link

Decodes analyzer frames and exports instrument errors to the LIS. C++20, CMake presets, GoogleTest.

## Build and test

- Build and test: `cmake --preset debug`, `cmake --build --preset debug`, `ctest --preset debug`.
- Done means: the same three commands pass for preset `debug` and for preset `tidy`.
  List the commands you ran and their result in your final summary.
- Never add `NOLINT` or `-Wno-*`, never edit `.clang-tidy`, `CMakePresets.json` or `third_party/`
  to get a build through. Tell the human what you would need instead.

## Domain rules (not visible in the code)

- `InstrumentError` values are wire codes: the firmware sends them and result archives store
  them. Never reorder, rename or insert. A new code is appended at the end and its value must
  equal the firmware code from the ticket.
- Every new error code or frame field is a protocol change: bump `kProtocolMinor` in
  `include/alink/protocol.hpp` by one. Firmware 3.2 speaks protocol 2.7.
- LIS flag `Q` (quality hold) for anything that makes already measured results suspect:
  temperature, expired or wrong reagent lot, calibration. `R` only for reagent supply
  (empty, lid open). If unsure, ask instead of guessing.
- `tests/data/recorded/` holds captures from real analyzers. Never edit them and never add
  hand-written frames there. New cases go to unit tests in `tests/*_test.cpp`. If a capture
  stops decoding as recorded, the code is wrong, not the capture.

## Recurring tasks

- New or changed instrument error code: use skill `add-instrument-error`.
