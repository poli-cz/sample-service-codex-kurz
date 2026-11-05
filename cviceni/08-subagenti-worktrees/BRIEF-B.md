# BRIEF: task B – JSON codec validation

**Goal:** `lab::io::sample_from_json` and `lab::io::measurement_from_json` reject values that the
rest of the service would never produce.

**Behaviour**
- `sample_from_json` with an id that `lab::is_valid_sample_id` rejects throws
  `std::invalid_argument` with the message `invalid sample id: <id>`.
- `measurement_from_json` with `position` below 1 throws `std::invalid_argument`.
- Round trips of valid samples and measurements keep working; update the comment in
  `json_codec.hpp` that lists the exceptions.

**Scope:** `src/labio/src/json_codec.cpp`, `src/labio/include/labio/json_codec.hpp` and
`tests/labio/json_codec_test.cpp` only.

**Process:** the `tester` agent writes failing tests first (tests/ only). Implementation starts
after a human has looked at the failures.

**Done:** `python scripts/check_all.py debug asan tidy` passes (tidy reports nothing), no new
dependencies, no `NOLINT`.
