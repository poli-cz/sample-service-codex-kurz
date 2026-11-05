#include "labcore/statistics.hpp"

#include <gtest/gtest.h>

#include <stdexcept>
#include <vector>

namespace lab::stats {
namespace {

TEST(Statistics, SampleStddevDividesByNMinusOne) {
  const std::vector<double> v{2.0, 4.0, 4.0, 4.0, 5.0, 5.0, 7.0, 9.0};
  EXPECT_DOUBLE_EQ(mean(v), 5.0);
  EXPECT_NEAR(sample_stddev(v), 2.138089935, 1e-9);
}

TEST(Statistics, StddevOfSingleValueIsZero) {
  const std::vector<double> v{3.0};
  EXPECT_DOUBLE_EQ(sample_stddev(v), 0.0);
}

TEST(Statistics, MedianOddAndEven) {
  EXPECT_DOUBLE_EQ(median(std::vector<double>{3.0, 1.0, 2.0}), 2.0);
  EXPECT_DOUBLE_EQ(median(std::vector<double>{4.0, 1.0, 3.0, 2.0}), 2.5);
}

TEST(Statistics, EmptyInputThrowsOrIsEmpty) {
  const std::vector<double> empty;
  EXPECT_THROW(static_cast<void>(mean(empty)), std::invalid_argument);
  EXPECT_THROW(static_cast<void>(median(empty)), std::invalid_argument);
  EXPECT_FALSE(summarize(empty).has_value());
}

TEST(Statistics, CvIsSdInPercentOfMeanOrZeroForZeroMean) {
  const std::vector<double> v{9.0, 10.0, 11.0};
  EXPECT_NEAR(cv_percent(v), 10.0, 1e-9);
  EXPECT_DOUBLE_EQ(cv_percent(std::vector<double>{-1.0, 1.0}), 0.0);
}

TEST(Statistics, SummaryHasCountMeanRangeAndMedian) {
  const std::vector<double> v{1.0, 2.0, 3.0, 4.0};
  const auto s = summarize(v);
  ASSERT_TRUE(s.has_value());
  EXPECT_EQ(s->count, 4U);
  EXPECT_DOUBLE_EQ(s->mean, 2.5);
  EXPECT_DOUBLE_EQ(s->min, 1.0);
  EXPECT_DOUBLE_EQ(s->max, 4.0);
  EXPECT_DOUBLE_EQ(s->median, 2.5);
}

TEST(Statistics, ZScoreIsSignedAndZeroWithoutPositiveSd) {
  EXPECT_DOUBLE_EQ(z_score(13.0, 10.0, 1.5), 2.0);
  EXPECT_DOUBLE_EQ(z_score(7.0, 10.0, 1.5), -2.0);
  EXPECT_DOUBLE_EQ(z_score(7.0, 10.0, 0.0), 0.0);
}

}  // namespace
}  // namespace lab::stats
