---
name: add-instrument-error
description: Use when adding or changing an instrument error code that the analyzer reports in ERR frames (new firmware error, new text, new LIS flag). Step-by-step procedure with the LIS flag table and the protocol version bump.
---

# Add an instrument error code

From the ticket you need: the firmware code number, the operator text and the firmware version.
If the code number is missing, stop and ask. Never invent a code.

1. Append the enumerator at the end of `InstrumentError` in `include/alink/instrument_error.hpp`
   and raise `kInstrumentErrorCount`. Check that its position equals the firmware code.
2. Add the operator text to `to_text` in `src/instrument_error.cpp`, exactly as in the ticket.
3. Choose the LIS flag in `lis_flag` (`src/lis_export.cpp`) by this table and say why in the summary:

   | the error means | flag |
   |---|---|
   | problem with the sample (barcode, clot, volume) | `S` |
   | reagent supply (empty, lid open), nothing measured yet | `R` |
   | hardware fault | `H` |
   | results already measured may be wrong (temperature, expired or wrong reagent lot, calibration) | `Q` |

4. Bump `kProtocolMinor` in `include/alink/protocol.hpp` by one.
5. Tests, only in `tests/instrument_error_test.cpp` and `tests/lis_export_test.cpp`:
   code to enum and back, text, LIS flag, and the next code still rejected.
   Do not touch `tests/data/recorded/`.
6. Run presets `debug` and `tidy` (build and ctest).

Final summary: code and enumerator, text, LIS flag with the reason, new protocol version,
commands you ran and their result.
