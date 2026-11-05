#include "labcore/sample.hpp"

#include <algorithm>
#include <cctype>

namespace lab {

std::string_view to_string(Priority priority) noexcept {
  switch (priority) {
    case Priority::Stat:
      return "STAT";
    case Priority::Urgent:
      return "URGENT";
    case Priority::Routine:
      return "ROUTINE";
  }
  return "UNKNOWN";
}

std::optional<Priority> parse_priority(std::string_view text) noexcept {
  if (text == "STAT" || text == "stat") {
    return Priority::Stat;
  }
  if (text == "URGENT" || text == "urgent") {
    return Priority::Urgent;
  }
  if (text == "ROUTINE" || text == "routine") {
    return Priority::Routine;
  }
  return std::nullopt;
}

bool is_valid_sample_id(std::string_view id) noexcept {
  constexpr std::size_t kPrefixLength = 2;
  constexpr std::size_t kMinDigits = 4;
  constexpr std::size_t kMaxDigits = 10;
  if (!id.starts_with("S-")) {
    return false;
  }
  const auto digits = id.substr(kPrefixLength);
  if (digits.size() < kMinDigits || digits.size() > kMaxDigits) {
    return false;
  }
  return std::ranges::all_of(
      digits, [](char c) { return std::isdigit(static_cast<unsigned char>(c)) != 0; });
}

}  // namespace lab
