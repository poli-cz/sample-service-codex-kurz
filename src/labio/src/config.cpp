#include "labio/config.hpp"

#include <fmt/format.h>

#include <fstream>
#include <nlohmann/json.hpp>
#include <sstream>

namespace lab::io {
namespace {

constexpr std::size_t kMaxRackSize = 120;

AssayConfig parse_assay(const std::string& code, const nlohmann::json& j) {
  AssayConfig assay;
  assay.code = code;
  assay.unit = j.at("unit").get<std::string>();
  const auto& range = j.at("reportable_range");
  assay.reportable_low = range.at(0).get<double>();
  assay.reportable_high = range.at(1).get<double>();
  if (assay.reportable_low >= assay.reportable_high) {
    throw ConfigError(fmt::format("assay {}: reportable_range must be increasing", code));
  }
  for (const auto& point : j.at("calibrators")) {
    assay.calibrators.push_back(
        {point.at("concentration").get<double>(), point.at("signal").get<double>()});
  }
  // Fail at startup, not at the first measurement.
  try {
    static_cast<void>(CalibrationCurve::fit(assay.calibrators));
  } catch (const CalibrationError& e) {
    throw ConfigError(fmt::format("assay {}: {}", code, e.what()));
  }
  return assay;
}

}  // namespace

ServiceConfig parse_config(std::string_view json_text) {
  nlohmann::json root;
  try {
    root = nlohmann::json::parse(json_text);
  } catch (const nlohmann::json::parse_error& e) {
    throw ConfigError(fmt::format("config is not valid JSON: {}", e.what()));
  }

  ServiceConfig config;
  try {
    config.rack_size = root.at("rack_size").get<std::size_t>();
    config.queue_capacity = root.at("queue_capacity").get<std::size_t>();
    config.instrument_seed = root.value("instrument_seed", std::uint32_t{1});
    for (const auto& [code, assay_json] : root.at("assays").items()) {
      config.assays.emplace(code, parse_assay(code, assay_json));
    }
  } catch (const nlohmann::json::exception& e) {
    throw ConfigError(fmt::format("config field missing or mistyped: {}", e.what()));
  }

  if (config.rack_size == 0 || config.rack_size > kMaxRackSize) {
    throw ConfigError(fmt::format("rack_size must be between 1 and {}", kMaxRackSize));
  }
  if (config.queue_capacity == 0) {
    throw ConfigError("queue_capacity must be positive");
  }
  if (config.assays.empty()) {
    throw ConfigError("at least one assay must be configured");
  }
  return config;
}

ServiceConfig load_config(const std::filesystem::path& path) {
  const std::ifstream in(path);
  if (!in) {
    throw ConfigError(fmt::format("cannot open config file {}", path.string()));
  }
  std::ostringstream buffer;
  buffer << in.rdbuf();
  return parse_config(buffer.str());
}

}  // namespace lab::io
