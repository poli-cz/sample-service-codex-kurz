#include "labio/config.hpp"

#include <gtest/gtest.h>

namespace lab::io {
namespace {

constexpr const char* kValid = R"({
  "rack_size": 4,
  "queue_capacity": 10,
  "assays": {
    "GLU": {
      "unit": "mmol/L",
      "reportable_range": [1.0, 30.0],
      "calibrators": [
        {"concentration": 0.0, "signal": 100.0},
        {"concentration": 30.0, "signal": 12100.0}
      ]
    }
  }
})";

TEST(Config, ParsesValidConfig) {
  const auto config = parse_config(kValid);
  EXPECT_EQ(config.rack_size, 4U);
  EXPECT_EQ(config.queue_capacity, 10U);
  EXPECT_EQ(config.instrument_seed, 1U);
  ASSERT_EQ(config.assays.count("GLU"), 1U);
  EXPECT_EQ(config.assays.at("GLU").calibrators.size(), 2U);
}

TEST(Config, MissingFieldIsReported) {
  EXPECT_THROW(static_cast<void>(parse_config(R"({"rack_size": 4})")), ConfigError);
}

}  // namespace
}  // namespace lab::io
