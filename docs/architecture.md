# Architecture

```
            +-------------------+
 commands ->| CommandProcessor  |-> JSON lines
            +---------+---------+
                      |
            +---------v---------+       +----------------------+
            |     Scheduler     |------>| SimulatedInstrument  |
            +--+------------+---+       +----------+-----------+
               |            |                      | firmware line
      +--------v---+   +----v-----+     +----------v-----------+
      | SampleQueue|   |   Rack   |     | parse_reading (labio)|
      +------------+   +----------+     |  -> legacy parser    |
                                        +----------------------+
```

## Flow of one sample

1. `SUBMIT` creates a `lab::Sample`; `Scheduler::submit` validates the assay and
   the id and puts it into `SampleQueue` (three lanes: STAT, URGENT, ROUTINE).
2. `LOAD` or `RUN` moves samples from the queue to free rack positions.
3. `RUN` asks `SimulatedInstrument` for one firmware line per rack position,
   for example `R|003|S-100001|GLU|2130.5|-|2026-01-01T00:00:00Z`.
4. `labio::parse_reading` decodes the line through the legacy C parser
   (`src/labio/legacy/`), which also understands firmware v1 lines.
5. `Scheduler::to_measurement` converts the signal to a concentration with the
   assay's `CalibrationCurve` and adds range flags (`L`, `H`, `D`).
6. `CommandProcessor` prints the result and keeps it for `REPORT` and `SUMMARY`.

## Firmware line format

| record | fields |
|---|---|
| result (v2) | `R|pos|sample|assay|signal|flags|timestamp` |
| control (v2) | `C|pos|lot|assay|signal|flags|timestamp` |
| error (v2) | `E|pos|sample|code|message` |
| heartbeat | `H|...` (ignored) |
| result (v1) | `R;pos;sample;assay;signal` |

Instrument flags: `HEM` (hemolysis), `CLOT`, `ERR`, `OK` (ignored), `-` for none.

## Calibration

Linear least squares, `signal = slope * concentration + intercept`, fitted at
startup from the calibrators in `config/service.json`. A fit with r squared below
0.98 or a non-positive slope stops the service at startup.
