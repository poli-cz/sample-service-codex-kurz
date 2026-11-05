#pragma once

#include <ostream>
#include <span>
#include <string>
#include <string_view>

#include "labcore/measurement.hpp"

namespace lab::io {

// Escapes one CSV field (RFC 4180 style).
[[nodiscard]] std::string csv_escape(std::string_view field);

// Writes a header line and one line per measurement.
void write_measurements_csv(std::ostream& out, std::span<const Measurement> measurements);

// Writes one line per assay: count, mean, sd, cv, flagged count.
void write_assay_summary_csv(std::ostream& out, std::span<const Measurement> measurements);

}  // namespace lab::io
