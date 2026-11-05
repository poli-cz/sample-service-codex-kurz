#include "labsched/instrument.hpp"

#include <gtest/gtest.h>

#include "labio/result_parser.hpp"
#include "test_config.hpp"

namespace lab::sched {
namespace {

TEST(SimulatedInstrument, IsDeterministic) {
  const auto config = testing::small_config();
  const SimulatedInstrument a(config);
  const SimulatedInstrument b(config);
  EXPECT_EQ(a.measure(1, "S-100001", "GLU"), b.measure(1, "S-100001", "GLU"));
}

TEST(SimulatedInstrument, ProducesParsableLines) {
  const auto config = testing::small_config();
  const SimulatedInstrument instrument(config);
  for (int i = 0; i < 200; ++i) {
    const auto id = "S-" + std::to_string(300000 + i);
    const auto outcome = io::parse_reading(instrument.measure(1 + i % 4, id, "K"));
    ASSERT_TRUE(std::holds_alternative<io::RawReading>(outcome)) << id;
  }
}

TEST(SimulatedInstrument, UnknownAssayGivesErrorRecord) {
  const auto config = testing::small_config();
  const SimulatedInstrument instrument(config);
  EXPECT_EQ(instrument.measure(2, "S-100001", "XYZ").rfind("E|002|", 0), 0U);
}

TEST(StableHash, DependsOnSalt) {
  EXPECT_NE(stable_hash("S-0001", 1), stable_hash("S-0001", 2));
  EXPECT_EQ(stable_hash("S-0001", 1), stable_hash("S-0001", 1));
}

}  // namespace
}  // namespace lab::sched
