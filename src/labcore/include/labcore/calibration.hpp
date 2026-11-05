#pragma once

#include <span>
#include <stdexcept>
#include <string>

namespace lab {

struct CalibrationPoint {
  double concentration = 0.0;
  double signal = 0.0;
};

class CalibrationError : public std::runtime_error {
 public:
  using std::runtime_error::runtime_error;
};

// Linear calibration: signal = slope * concentration + intercept.
// Fitted by ordinary least squares from calibrator measurements.
class CalibrationCurve {
 public:
  // Throws CalibrationError when fewer than two distinct concentrations are given,
  // when the slope is not positive, or when r squared is below min_r_squared.
  static CalibrationCurve fit(std::span<const CalibrationPoint> points,
                              double min_r_squared = 0.98);

  CalibrationCurve(double slope, double intercept, double low_signal, double high_signal);

  [[nodiscard]] double slope() const noexcept { return slope_; }
  [[nodiscard]] double intercept() const noexcept { return intercept_; }
  [[nodiscard]] double r_squared() const noexcept { return r_squared_; }

  [[nodiscard]] double signal_for(double concentration) const noexcept;
  [[nodiscard]] double concentration_for(double signal) const noexcept;

  // Signals outside the calibrated range are extrapolated; callers flag them.
  [[nodiscard]] bool in_calibrated_range(double signal) const noexcept;

 private:
  double slope_;
  double intercept_;
  double low_signal_;
  double high_signal_;
  double r_squared_ = 1.0;
};

}  // namespace lab
