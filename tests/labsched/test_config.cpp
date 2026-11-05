#include "test_config.hpp"

namespace lab::sched::testing {

io::ServiceConfig small_config() {
  return io::parse_config(R"({
    "rack_size": 4,
    "queue_capacity": 20,
    "instrument_seed": 7,
    "assays": {
      "GLU": {
        "unit": "mmol/L",
        "reportable_range": [1.0, 30.0],
        "calibrators": [
          {"concentration": 0.0, "signal": 120.0},
          {"concentration": 15.0, "signal": 6090.0},
          {"concentration": 30.0, "signal": 12150.0}
        ]
      },
      "K": {
        "unit": "mmol/L",
        "reportable_range": [1.5, 10.0],
        "calibrators": [
          {"concentration": 1.0, "signal": 510.0},
          {"concentration": 10.0, "signal": 5020.0}
        ]
      }
    }
  })");
}

}  // namespace lab::sched::testing
