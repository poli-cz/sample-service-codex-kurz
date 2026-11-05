#include "labio/result_parser.hpp"

#include <string>

#include "legacy_result_parser.h"

namespace lab::io {
namespace {

ResultFlags translate_flags(unsigned int legacy_flags) {
  ResultFlags flags = 0;
  if ((legacy_flags & LR_F_HEM) != 0U) {
    add_flag(flags, ResultFlag::Hemolysis);
  }
  if ((legacy_flags & LR_F_CLOT) != 0U) {
    add_flag(flags, ResultFlag::Clot);
  }
  if ((legacy_flags & LR_F_ERR) != 0U) {
    add_flag(flags, ResultFlag::InstrumentError);
  }
  return flags;
}

RecordKind translate_kind(char type) {
  switch (type) {
    case 'C':
      return RecordKind::Control;
    case 'E':
      return RecordKind::Error;
    default:
      return RecordKind::Result;
  }
}

}  // namespace

ParseOutcome parse_reading(std::string_view line) {
  const std::string owned(line);  // the legacy parser needs a terminated string
  LEGACY_RESULT raw{};
  const int rc = lr_parse(owned.c_str(), &raw);
  if (rc == LR_SKIP) {
    return Skipped{};
  }
  if (rc != LR_OK) {
    return ParseError{lr_errstr(rc)};
  }

  RawReading reading;
  reading.kind = translate_kind(raw.type);
  reading.rack_position = raw.pos;
  reading.sample_id = static_cast<const char*>(raw.sid);
  reading.assay = static_cast<const char*>(raw.assay);
  reading.signal = raw.signal;
  reading.flags = translate_flags(raw.flags);
  reading.error_code = raw.errcode;
  reading.message = static_cast<const char*>(raw.msg);
  reading.firmware_version = raw.fw;
  return reading;
}

}  // namespace lab::io
