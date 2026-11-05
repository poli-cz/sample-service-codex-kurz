# BRIEF: task A – config validation

**Goal:** `lab::io::parse_config` rejects two more kinds of broken assay configuration.

**Behaviour**
- An assay with an empty `unit` throws `ConfigError` with the message
  `assay <CODE>: unit must not be empty`.
- A `reportable_range` that is not an array of exactly two numbers throws `ConfigError` with the
  message `assay <CODE>: reportable_range must have exactly two values`.
- Everything that is valid today stays valid; the existing messages do not change.

**Scope:** `src/labio/src/config.cpp` and `tests/labio/config_test.cpp` only.

**Process:** the `tester` agent writes failing tests first (tests/ only). Implementation starts
after a human has looked at the failures.

**Done:** `python scripts/check_all.py debug asan tidy` passes (tidy reports nothing), no new
dependencies, no `NOLINT`.
