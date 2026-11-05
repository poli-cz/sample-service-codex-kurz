#include "labcore/qc_rules.hpp"

#include <gtest/gtest.h>

#include <stdexcept>
#include <vector>

namespace lab::qc {
namespace {

constexpr double kMean = 100.0;
constexpr double kSd = 10.0;

std::vector<Violation> evaluate(const std::vector<double>& values) {
  return evaluate_westgard(values, kMean, kSd);
}

TEST(Westgard, EmptyInputHasNoViolations) { EXPECT_TRUE(evaluate({}).empty()); }

TEST(Westgard, ValuesWithinTwoSdHaveNoViolations) {
  EXPECT_TRUE(evaluate({100.0, 119.9, 80.1, 105.0}).empty());
}

TEST(Westgard, OneValueBeyondTwoSdIsAWarning) {
  EXPECT_EQ(evaluate({100.0, 125.0}), (std::vector<Violation>{{"1-2s", 1}}));
}

TEST(Westgard, OneValueBeyondThreeSdAlsoReportsTheWarning) {
  EXPECT_EQ(evaluate({131.0}), (std::vector<Violation>{{"1-2s", 0}, {"1-3s", 0}}));
  EXPECT_EQ(evaluate({69.0}), (std::vector<Violation>{{"1-2s", 0}, {"1-3s", 0}}));
}

TEST(Westgard, TwoConsecutiveBeyondTheSameTwoSdLimit) {
  EXPECT_EQ(evaluate({121.0, 122.0}),
            (std::vector<Violation>{{"1-2s", 0}, {"1-2s", 1}, {"2-2s", 1}}));
  EXPECT_EQ(evaluate({79.0, 78.0}),
            (std::vector<Violation>{{"1-2s", 0}, {"1-2s", 1}, {"2-2s", 1}}));
}

TEST(Westgard, RangeRuleNeedsOppositeTwoSdLimits) {
  EXPECT_EQ(evaluate({121.0, 78.0}),
            (std::vector<Violation>{{"1-2s", 0}, {"1-2s", 1}, {"R-4s", 1}}));
  EXPECT_EQ(evaluate({78.0, 121.0}),
            (std::vector<Violation>{{"1-2s", 0}, {"1-2s", 1}, {"R-4s", 1}}));
}

TEST(Westgard, FourConsecutiveBeyondTheSameOneSdLimit) {
  EXPECT_EQ(evaluate({111.0, 112.0, 113.0, 114.0}), (std::vector<Violation>{{"4-1s", 3}}));
  EXPECT_EQ(evaluate({89.0, 88.0, 87.0, 86.0}), (std::vector<Violation>{{"4-1s", 3}}));
}

TEST(Westgard, RunLongerThanTheWindowFiresAtEveryIndex) {
  EXPECT_EQ(evaluate({111.0, 112.0, 113.0, 114.0, 115.0}),
            (std::vector<Violation>{{"4-1s", 3}, {"4-1s", 4}}));
}

TEST(Westgard, FourValuesOnOppositeSidesAreNotAFourOneSdRun) {
  EXPECT_TRUE(evaluate({111.0, 112.0, 88.0, 114.0}).empty());
}

TEST(Westgard, TenConsecutiveOnTheSameSideOfTheMean) {
  const std::vector<double> rising{101, 102, 103, 104, 105, 106, 107, 108, 109, 110};
  // 110 is exactly M + 1S, so it does not complete a 4-1s run.
  EXPECT_EQ(evaluate(rising), (std::vector<Violation>{{"10-x", 9}}));
  const std::vector<double> below{99, 98, 97, 96, 95, 95, 96, 97, 98, 99};
  EXPECT_EQ(evaluate(below), (std::vector<Violation>{{"10-x", 9}}));
}

TEST(Westgard, ValueOnTheMeanBreaksATenValueRun) {
  EXPECT_TRUE(evaluate({101, 102, 103, 104, 105, 106, 107, 108, 109, 100}).empty());
}

TEST(Westgard, ValuesExactlyOnALimitDoNotExceedIt) {
  EXPECT_TRUE(evaluate({120.0}).empty());
  EXPECT_TRUE(evaluate({80.0}).empty());
  EXPECT_EQ(evaluate({130.0}), (std::vector<Violation>{{"1-2s", 0}}));
  EXPECT_TRUE(evaluate({110.0, 110.0, 110.0, 110.0}).empty());
  EXPECT_TRUE(evaluate({120.0, 120.0}).empty());
}

TEST(Westgard, ResultsAreOrderedByIndexThenByRule) {
  EXPECT_EQ(evaluate({131.0, 132.0}),
            (std::vector<Violation>{
                {"1-2s", 0}, {"1-3s", 0}, {"1-2s", 1}, {"1-3s", 1}, {"2-2s", 1}}));
}

TEST(Westgard, NonPositiveSdIsRejected) {
  const std::vector<double> values{100.0};
  EXPECT_THROW((void)evaluate_westgard(values, kMean, 0.0), std::invalid_argument);
  EXPECT_THROW((void)evaluate_westgard(values, kMean, -1.0), std::invalid_argument);
}

}  // namespace
}  // namespace lab::qc
