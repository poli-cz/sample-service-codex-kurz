#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <map>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "labcore/calibration.hpp"

namespace lab::io {

struct AssayConfig {
  std::string code;
  std::string unit;
  double reportable_low = 0.0;
  double reportable_high = 0.0;
  std::vector<CalibrationPoint> calibrators;
};

struct ServiceConfig {
  std::size_t rack_size = 0;
  std::size_t queue_capacity = 0;
  std::uint32_t instrument_seed = 0;
  std::map<std::string, AssayConfig, std::less<>> assays;
};

class ConfigError : public std::runtime_error {
 public:
  using std::runtime_error::runtime_error;
};

// Throws ConfigError with a message that names the offending field.
[[nodiscard]] ServiceConfig parse_config(std::string_view json_text);
[[nodiscard]] ServiceConfig load_config(const std::filesystem::path& path);

}  // namespace lab::io
