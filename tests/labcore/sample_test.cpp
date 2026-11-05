#include "labcore/sample.hpp"

#include <gtest/gtest.h>

#include "labcore/measurement.hpp"

namespace lab {
namespace {

TEST(SampleId, AcceptsPrefixAndFourToTenDigits) {
  EXPECT_TRUE(is_valid_sample_id("S-0001"));
  EXPECT_TRUE(is_valid_sample_id("S-1234567890"));
}

TEST(SampleId, RejectsMalformedIds) {
  EXPECT_FALSE(is_valid_sample_id(""));
  EXPECT_FALSE(is_valid_sample_id("S-123"));
  EXPECT_FALSE(is_valid_sample_id("S-12345678901"));
  EXPECT_FALSE(is_valid_sample_id("X-1234"));
  EXPECT_FALSE(is_valid_sample_id("S-12a4"));
  EXPECT_FALSE(is_valid_sample_id("s-1234"));
}

TEST(Priority, RoundTripsThroughText) {
  for (const auto p : {Priority::Stat, Priority::Urgent, Priority::Routine}) {
    EXPECT_EQ(parse_priority(to_string(p)), p);
  }
  EXPECT_EQ(parse_priority("stat"), Priority::Stat);
  EXPECT_FALSE(parse_priority("Emergency").has_value());
}

TEST(ResultFlags, CodesFollowDeclarationOrder) {
  const ResultFlags flags = ResultFlag::Dilute | ResultFlag::High;
  const auto codes = flag_codes(flags);
  ASSERT_EQ(codes.size(), 2U);
  EXPECT_EQ(codes[0], "H");
  EXPECT_EQ(codes[1], "D");
  EXPECT_TRUE(flag_codes(0).empty());
}

}  // namespace
}  // namespace lab
