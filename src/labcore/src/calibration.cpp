#include "labcore/calibration.hpp"

#include <algorithm>
#include <cmath>
#include <set>

namespace lab {

CalibrationCurve::CalibrationCurve(double slope, double intercept, double low_signal,
                                   double high_signal)
    : slope_(slope), intercept_(intercept), low_signal_(low_signal), high_signal_(high_signal) {
  if (!(slope_ > 0.0)) {
    throw CalibrationError("calibration slope must be positive");
  }
  if (low_signal_ > high_signal_) {
    std::swap(low_signal_, high_signal_);
  }
}

CalibrationCurve CalibrationCurve::fit(std::span<const CalibrationPoint> points,
                                       double min_r_squared) {
  std::set<double> distinct;
  for (const auto& p : points) {
    distinct.insert(p.concentration);
  }
  if (distinct.size() < 2) {
    throw CalibrationError("calibration needs at least two distinct concentrations");
  }

  const auto n = static_cast<double>(points.size());
  double sum_x = 0.0;
  double sum_y = 0.0;
  for (const auto& p : points) {
    sum_x += p.concentration;
    sum_y += p.signal;
  }
  const double mean_x = sum_x / n;
  const double mean_y = sum_y / n;

  double sxx = 0.0;
  double sxy = 0.0;
  double syy = 0.0;
  for (const auto& p : points) {
    const double dx = p.concentration - mean_x;
    const double dy = p.signal - mean_y;
    sxx += dx * dx;
    sxy += dx * dy;
    syy += dy * dy;
  }

  const double slope = sxy / sxx;
  const double intercept = mean_y - (slope * mean_x);
  const double r_squared = syy == 0.0 ? 0.0 : (sxy * sxy) / (sxx * syy);
  if (r_squared < min_r_squared) {
    throw CalibrationError("calibration r squared " + std::to_string(r_squared) +
                           " is below the limit " + std::to_string(min_r_squared));
  }

  const auto [lowest, highest] = std::ranges::minmax(points, {}, &CalibrationPoint::signal);

  CalibrationCurve curve(slope, intercept, lowest.signal, highest.signal);
  curve.r_squared_ = r_squared;
  return curve;
}

double CalibrationCurve::signal_for(double concentration) const noexcept {
  return (slope_ * concentration) + intercept_;
}

double CalibrationCurve::concentration_for(double signal) const noexcept {
  return (signal - intercept_) / slope_;
}

bool CalibrationCurve::in_calibrated_range(double signal) const noexcept {
  return signal >= low_signal_ && signal <= high_signal_;
}

}  // namespace lab
