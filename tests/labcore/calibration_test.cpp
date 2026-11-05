#include "labcore/calibration.hpp"

#include <gtest/gtest.h>

#include <vector>

namespace lab {
namespace {

TEST(CalibrationCurve, FitsAnExactLine) {
  const std::vector<CalibrationPoint> points{{0.0, 100.0}, {10.0, 600.0}, {20.0, 1100.0}};
  const auto curve = CalibrationCurve::fit(points);
  EXPECT_DOUBLE_EQ(curve.slope(), 50.0);
  EXPECT_DOUBLE_EQ(curve.intercept(), 100.0);
  EXPECT_DOUBLE_EQ(curve.r_squared(), 1.0);
}

TEST(CalibrationCurve, InverseRoundTrips) {
  const std::vector<CalibrationPoint> points{{1.0, 510.0}, {4.0, 2005.0}, {10.0, 5020.0}};
  const auto curve = CalibrationCurve::fit(points);
  for (const double c : {1.5, 4.2, 9.9}) {
    EXPECT_NEAR(curve.concentration_for(curve.signal_for(c)), c, 1e-9);
  }
}

TEST(CalibrationCurve, NeedsTwoDistinctConcentrations) {
  const std::vector<CalibrationPoint> points{{5.0, 100.0}, {5.0, 110.0}};
  EXPECT_THROW(static_cast<void>(CalibrationCurve::fit(points)), CalibrationError);
}

TEST(CalibrationCurve, RejectsPoorFit) {
  const std::vector<CalibrationPoint> points{{0.0, 100.0}, {10.0, 900.0}, {20.0, 150.0}};
  EXPECT_THROW(static_cast<void>(CalibrationCurve::fit(points)), CalibrationError);
}

TEST(CalibrationCurve, RejectsNonPositiveSlope) {
  EXPECT_THROW(CalibrationCurve(0.0, 1.0, 0.0, 10.0), CalibrationError);
  EXPECT_THROW(CalibrationCurve(-2.0, 1.0, 0.0, 10.0), CalibrationError);
}

TEST(CalibrationCurve, KnowsItsCalibratedRange) {
  const std::vector<CalibrationPoint> points{{0.0, 100.0}, {10.0, 600.0}, {20.0, 1100.0}};
  const auto curve = CalibrationCurve::fit(points);
  EXPECT_TRUE(curve.in_calibrated_range(100.0));
  EXPECT_TRUE(curve.in_calibrated_range(1100.0));
  EXPECT_FALSE(curve.in_calibrated_range(99.9));
  EXPECT_FALSE(curve.in_calibrated_range(1100.1));
}

}  // namespace
}  // namespace lab
