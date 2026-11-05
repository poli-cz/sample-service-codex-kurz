#pragma once

#include <nlohmann/json.hpp>

#include "labcore/measurement.hpp"
#include "labcore/sample.hpp"

namespace lab::io {

[[nodiscard]] nlohmann::json to_json(const Sample& sample);
[[nodiscard]] nlohmann::json to_json(const Measurement& measurement);

// Throws nlohmann::json::exception on missing or mistyped fields and
// std::invalid_argument on an unknown priority or flag code.
[[nodiscard]] Sample sample_from_json(const nlohmann::json& j);
[[nodiscard]] Measurement measurement_from_json(const nlohmann::json& j);

}  // namespace lab::io
