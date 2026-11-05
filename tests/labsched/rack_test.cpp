#include "labsched/rack.hpp"

#include <gtest/gtest.h>

namespace lab::sched {
namespace {

Sample make(std::string id) { return Sample{std::move(id), "GLU", Priority::Routine, 0}; }

TEST(Rack, LoadsIntoLowestFreePosition) {
  Rack rack(3);
  EXPECT_EQ(rack.load(make("S-0001")), 1);
  EXPECT_EQ(rack.load(make("S-0002")), 2);
  EXPECT_TRUE(rack.unload(1));
  EXPECT_EQ(rack.load(make("S-0003")), 1);
  EXPECT_EQ(rack.load(make("S-0004")), 3);
  EXPECT_TRUE(rack.full());
  EXPECT_FALSE(rack.load(make("S-0005")).has_value());
}

TEST(Rack, UnloadAllReturnsSlotsInPositionOrder) {
  Rack rack(3);
  rack.load_at(3, make("S-0003"));
  rack.load_at(1, make("S-0001"));
  const auto slots = rack.unload_all();
  ASSERT_EQ(slots.size(), 2U);
  EXPECT_EQ(slots[0].position, 1);
  EXPECT_EQ(slots[1].position, 3);
  EXPECT_TRUE(rack.empty());
}

TEST(Rack, LoadAtRejectsTakenOrInvalidPositions) {
  Rack rack(2);
  EXPECT_TRUE(rack.load_at(2, make("S-0001")));
  EXPECT_FALSE(rack.load_at(2, make("S-0002")));
  EXPECT_FALSE(rack.load_at(0, make("S-0002")));
  EXPECT_FALSE(rack.load_at(3, make("S-0002")));
}

}  // namespace
}  // namespace lab::sched
