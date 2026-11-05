#include "labcore/qc_rules.hpp"

#include <algorithm>
#include <array>
#include <stdexcept>

namespace lab::qc {
namespace {

// Control limits are M +- k*S of the control lot. "Beyond" is strict: a value exactly on
// a limit stays inside it, a value exactly on the mean is on neither side.
bool above(double value, double mean, double sd, double k) { return value > mean + (k * sd); }
bool below(double value, double mean, double sd, double k) { return value < mean - (k * sd); }

// True when all values of the window are beyond the same k*S limit.
bool all_beyond_same_limit(std::span<const double> window, double mean, double sd, double k) {
  return std::ranges::all_of(window, [&](double x) { return above(x, mean, sd, k); }) ||
         std::ranges::all_of(window, [&](double x) { return below(x, mean, sd, k); });
}

// One rule: its name, how many values it looks at, and the test on that window.
struct Rule {
  const char* name;
  std::size_t window;
  bool (*fires)(std::span<const double> window, double mean, double sd);
};

constexpr std::size_t kTenX = 10;

// In the order the results are reported within one index.
constexpr std::array<Rule, 6> kRules{{
    {"1-2s", 1,
     [](std::span<const double> w, double m, double s) {
       return above(w[0], m, s, 2.0) || below(w[0], m, s, 2.0);
     }},
    {"1-3s", 1,
     [](std::span<const double> w, double m, double s) {
       return above(w[0], m, s, 3.0) || below(w[0], m, s, 3.0);
     }},
    {"2-2s", 2,
     [](std::span<const double> w, double m, double s) {
       return all_beyond_same_limit(w, m, s, 2.0);
     }},
    {"R-4s", 2,
     [](std::span<const double> w, double m, double s) {
       return (above(w[0], m, s, 2.0) && below(w[1], m, s, 2.0)) ||
              (below(w[0], m, s, 2.0) && above(w[1], m, s, 2.0));
     }},
    {"4-1s", 4,
     [](std::span<const double> w, double m, double s) {
       return all_beyond_same_limit(w, m, s, 1.0);
     }},
    {"10-x", kTenX,
     [](std::span<const double> w, double m, double s) {
       return all_beyond_same_limit(w, m, s, 0.0);
     }},
}};

}  // namespace

std::vector<Violation> evaluate_westgard(std::span<const double> values, double target_mean,
                                         double target_sd) {
  if (!(target_sd > 0.0)) {
    throw std::invalid_argument("target SD must be positive");
  }
  std::vector<Violation> out;
  for (std::size_t i = 0; i < values.size(); ++i) {
    for (const auto& rule : kRules) {
      if (i + 1 < rule.window) {
        continue;
      }
      const auto window = values.subspan(i + 1 - rule.window, rule.window);
      if (rule.fires(window, target_mean, target_sd)) {
        out.push_back({.rule = rule.name, .index = i});
      }
    }
  }
  return out;
}

}  // namespace lab::qc
