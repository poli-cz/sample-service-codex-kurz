#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace lab {

// Result flags are a bit set so that the instrument and the service can both add flags.
enum class ResultFlag : std::uint8_t {
  None = 0,
  High = 1U << 0U,          // above the reportable range
  Low = 1U << 1U,           // below the reportable range
  Dilute = 1U << 2U,        // far above the range, rerun with dilution
  InstrumentError = 1U << 3U,
  Hemolysis = 1U << 4U,     // reported by the instrument
  Clot = 1U << 5U,          // reported by the instrument
};

using ResultFlags = std::uint32_t;

[[nodiscard]] constexpr ResultFlags operator|(ResultFlag a, ResultFlag b) noexcept {
  return static_cast<ResultFlags>(a) | static_cast<ResultFlags>(b);
}

[[nodiscard]] constexpr bool has_flag(ResultFlags flags, ResultFlag flag) noexcept {
  return (flags & static_cast<ResultFlags>(flag)) != 0U;
}

constexpr void add_flag(ResultFlags& flags, ResultFlag flag) noexcept {
  flags |= static_cast<ResultFlags>(flag);
}

// Short codes used in the text and CSV outputs, e.g. "H,D".
[[nodiscard]] std::vector<std::string_view> flag_codes(ResultFlags flags);

struct Measurement {
  std::string sample_id;
  std::string assay;
  int rack_position = 0;
  double raw_signal = 0.0;
  double concentration = 0.0;
  std::string unit;
  ResultFlags flags = 0;
};

}  // namespace lab
