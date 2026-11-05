#include "labio/csv_report.hpp"

#include <fmt/format.h>

#include <map>
#include <string>
#include <vector>

#include "labcore/statistics.hpp"

namespace lab::io {
namespace {

std::string join_flags(ResultFlags flags) {
  std::string out;
  for (const auto code : flag_codes(flags)) {
    if (!out.empty()) {
      out += ';';
    }
    out += code;
  }
  return out;
}

}  // namespace

std::string csv_escape(std::string_view field) {
  const bool needs_quotes = field.find_first_of(",\"\n") != std::string_view::npos;
  if (!needs_quotes) {
    return std::string(field);
  }
  return fmt::format("\"{}\"", field);
}

void write_measurements_csv(std::ostream& out, std::span<const Measurement> measurements) {
  out << "sample_id,assay,position,signal,concentration,unit,flags\n";
  for (const auto& m : measurements) {
    out << fmt::format("{},{},{},{:.1f},{:.3f},{},{}\n", csv_escape(m.sample_id),
                       csv_escape(m.assay), m.rack_position, m.raw_signal, m.concentration,
                       csv_escape(m.unit), csv_escape(join_flags(m.flags)));
  }
}

void write_assay_summary_csv(std::ostream& out, std::span<const Measurement> measurements) {
  struct Bucket {
    std::vector<double> values;
    std::size_t flagged = 0;
    std::string unit;
  };
  std::map<std::string, Bucket, std::less<>> buckets;
  for (const auto& m : measurements) {
    auto& bucket = buckets[m.assay];
    bucket.unit = m.unit;
    if (m.flags != 0) {
      ++bucket.flagged;
    }
    if (!has_flag(m.flags, ResultFlag::InstrumentError)) {
      bucket.values.push_back(m.concentration);
    }
  }

  out << "assay,unit,count,mean,sd,cv_percent,flagged\n";
  for (const auto& [assay, bucket] : buckets) {
    const auto summary = stats::summarize(bucket.values);
    if (!summary) {
      out << fmt::format("{},{},0,,,,{}\n", csv_escape(assay), csv_escape(bucket.unit),
                         bucket.flagged);
      continue;
    }
    out << fmt::format("{},{},{},{:.3f},{:.3f},{:.2f},{}\n", csv_escape(assay),
                       csv_escape(bucket.unit), summary->count, summary->mean, summary->stddev,
                       summary->cv_percent, bucket.flagged);
  }
}

}  // namespace lab::io
