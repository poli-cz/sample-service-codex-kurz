#include "labcore/statistics.hpp"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <stdexcept>
#include <vector>

namespace lab::stats {

double mean(std::span<const double> values) {
  if (values.empty()) {
    throw std::invalid_argument("mean of an empty range");
  }
  return std::accumulate(values.begin(), values.end(), 0.0) / static_cast<double>(values.size());
}

double sample_stddev(std::span<const double> values) {
  if (values.size() < 2) {
    return 0.0;
  }
  const double m = mean(values);
  double squares = 0.0;
  for (const double v : values) {
    squares += (v - m) * (v - m);
  }
  return std::sqrt(squares / static_cast<double>(values.size() - 1));
}

double median(std::span<const double> values) {
  if (values.empty()) {
    throw std::invalid_argument("median of an empty range");
  }
  std::vector<double> sorted(values.begin(), values.end());
  std::ranges::sort(sorted);
  const std::size_t mid = sorted.size() / 2;
  if (sorted.size() % 2 == 0) {
    return (sorted[mid - 1] + sorted[mid]) / 2.0;
  }
  return sorted[mid];
}

double cv_percent(std::span<const double> values) {
  const double m = mean(values);
  if (m == 0.0) {
    return 0.0;
  }
  constexpr double kPercent = 100.0;
  return sample_stddev(values) / std::abs(m) * kPercent;
}

std::optional<Summary> summarize(std::span<const double> values) {
  if (values.empty()) {
    return std::nullopt;
  }
  const auto [lowest, highest] = std::ranges::minmax(values);
  Summary s;
  s.count = values.size();
  s.mean = mean(values);
  s.stddev = sample_stddev(values);
  s.cv_percent = cv_percent(values);
  s.min = lowest;
  s.max = highest;
  s.median = median(values);
  return s;
}

double z_score(double value, double target_mean, double target_sd) noexcept {
  if (target_sd <= 0.0) {
    return 0.0;
  }
  return (value - target_mean) / target_sd;
}

}  // namespace lab::stats
