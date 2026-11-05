#pragma once

#include <cstddef>
#include <span>
#include <string>
#include <vector>

namespace lab::qc {

// One Westgard rule that fired. `index` is the value that completed the rule.
struct Violation {
  std::string rule;  // "1-2s", "1-3s", "2-2s", "R-4s", "4-1s" or "10-x"
  std::size_t index = 0;
  friend bool operator==(const Violation&, const Violation&) = default;
};

// Evaluates the Westgard multirule scheme on control values in measurement order.
// "Beyond" a limit means strictly greater (or strictly less); a value on the mean is on
// neither side. Results are ordered by index, then by rule in the order listed above.
// Throws std::invalid_argument when target_sd <= 0.
[[nodiscard]] std::vector<Violation> evaluate_westgard(std::span<const double> values,
                                                       double target_mean, double target_sd);

}  // namespace lab::qc
