#pragma once

#include <cstddef>
#include <optional>
#include <span>

namespace lab::stats {

struct Summary {
  std::size_t count = 0;
  double mean = 0.0;
  double stddev = 0.0;  // sample standard deviation (n - 1), 0 for a single value
  double cv_percent = 0.0;
  double min = 0.0;
  double max = 0.0;
  double median = 0.0;
};

[[nodiscard]] double mean(std::span<const double> values);
[[nodiscard]] double sample_stddev(std::span<const double> values);
[[nodiscard]] double median(std::span<const double> values);

// Coefficient of variation in percent; 0 when the mean is 0.
[[nodiscard]] double cv_percent(std::span<const double> values);

// Empty optional for an empty input.
[[nodiscard]] std::optional<Summary> summarize(std::span<const double> values);

// z-score of value against a target mean and standard deviation.
[[nodiscard]] double z_score(double value, double target_mean, double target_sd) noexcept;

}  // namespace lab::stats
