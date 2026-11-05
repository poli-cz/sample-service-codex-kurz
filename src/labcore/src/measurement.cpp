#include "labcore/measurement.hpp"

#include <array>
#include <utility>

namespace lab {

std::vector<std::string_view> flag_codes(ResultFlags flags) {
  static constexpr std::array<std::pair<ResultFlag, std::string_view>, 6> kCodes{{
      {ResultFlag::High, "H"},
      {ResultFlag::Low, "L"},
      {ResultFlag::Dilute, "D"},
      {ResultFlag::InstrumentError, "E"},
      {ResultFlag::Hemolysis, "HEM"},
      {ResultFlag::Clot, "CLOT"},
  }};
  std::vector<std::string_view> codes;
  for (const auto& [flag, code] : kCodes) {
    if (has_flag(flags, flag)) {
      codes.push_back(code);
    }
  }
  return codes;
}

}  // namespace lab
