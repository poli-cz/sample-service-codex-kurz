#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <variant>

#include "labcore/measurement.hpp"

namespace lab::io {

enum class RecordKind : std::uint8_t { Result, Control, Error };

// One line from the analyzer, decoded.
struct RawReading {
  RecordKind kind = RecordKind::Result;
  int rack_position = 0;
  std::string sample_id;  // control lot for RecordKind::Control
  std::string assay;      // empty for RecordKind::Error
  double signal = 0.0;
  ResultFlags flags = 0;  // instrument flags only (Hemolysis, Clot, InstrumentError)
  int error_code = 0;
  std::string message;
  int firmware_version = 2;
};

struct Skipped {};  // heartbeat or comment line

struct ParseError {
  std::string message;
};

using ParseOutcome = std::variant<RawReading, Skipped, ParseError>;

// Parses one analyzer line (firmware v1 or v2). Thread-compatible only:
// the legacy implementation underneath uses static buffers.
[[nodiscard]] ParseOutcome parse_reading(std::string_view line);

}  // namespace lab::io
