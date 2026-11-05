#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace lab {

// Lower value = served first.
enum class Priority : std::uint8_t { Stat = 0, Urgent = 1, Routine = 2 };

[[nodiscard]] std::string_view to_string(Priority priority) noexcept;
[[nodiscard]] std::optional<Priority> parse_priority(std::string_view text) noexcept;

struct Sample {
  std::string id;     // "S-" followed by 4 to 10 digits, e.g. "S-004711"
  std::string assay;  // assay code from the service configuration, e.g. "GLU"
  Priority priority = Priority::Routine;
  std::uint64_t sequence = 0;  // assigned by SampleQueue, keeps FIFO order within a priority
};

[[nodiscard]] bool is_valid_sample_id(std::string_view id) noexcept;

}  // namespace lab
