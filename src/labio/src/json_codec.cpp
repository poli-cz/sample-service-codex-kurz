#include "labio/json_codec.hpp"

#include <stdexcept>
#include <string>

namespace lab::io {
namespace {

ResultFlags flags_from_codes(const nlohmann::json& codes) {
  ResultFlags flags = 0;
  for (const auto& code : codes) {
    const auto text = code.get<std::string>();
    if (text == "H") {
      add_flag(flags, ResultFlag::High);
    } else if (text == "L") {
      add_flag(flags, ResultFlag::Low);
    } else if (text == "D") {
      add_flag(flags, ResultFlag::Dilute);
    } else if (text == "E") {
      add_flag(flags, ResultFlag::InstrumentError);
    } else if (text == "HEM") {
      add_flag(flags, ResultFlag::Hemolysis);
    } else if (text == "CLOT") {
      add_flag(flags, ResultFlag::Clot);
    } else {
      throw std::invalid_argument("unknown flag code: " + text);
    }
  }
  return flags;
}

}  // namespace

nlohmann::json to_json(const Sample& sample) {
  return {
      {"id", sample.id},
      {"assay", sample.assay},
      {"priority", std::string(to_string(sample.priority))},
      {"sequence", sample.sequence},
  };
}

nlohmann::json to_json(const Measurement& measurement) {
  nlohmann::json flags = nlohmann::json::array();
  for (const auto code : flag_codes(measurement.flags)) {
    flags.push_back(std::string(code));
  }
  return {
      {"sample_id", measurement.sample_id},
      {"assay", measurement.assay},
      {"position", measurement.rack_position},
      {"signal", measurement.raw_signal},
      {"concentration", measurement.concentration},
      {"unit", measurement.unit},
      {"flags", flags},
  };
}

Sample sample_from_json(const nlohmann::json& j) {
  Sample sample;
  sample.id = j.at("id").get<std::string>();
  sample.assay = j.at("assay").get<std::string>();
  const auto priority_text = j.at("priority").get<std::string>();
  const auto priority = parse_priority(priority_text);
  if (!priority) {
    throw std::invalid_argument("unknown priority: " + priority_text);
  }
  sample.priority = *priority;
  sample.sequence = j.value("sequence", std::uint64_t{0});
  return sample;
}

Measurement measurement_from_json(const nlohmann::json& j) {
  Measurement m;
  m.sample_id = j.at("sample_id").get<std::string>();
  m.assay = j.at("assay").get<std::string>();
  m.rack_position = j.at("position").get<int>();
  m.raw_signal = j.at("signal").get<double>();
  m.concentration = j.at("concentration").get<double>();
  m.unit = j.at("unit").get<std::string>();
  m.flags = flags_from_codes(j.at("flags"));
  return m;
}

}  // namespace lab::io
