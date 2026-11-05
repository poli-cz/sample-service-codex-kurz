#include "labio/result_parser.hpp"

#include <gtest/gtest.h>

namespace lab::io {
namespace {

TEST(ResultParser, ParsesFirmwareV2Result) {
  const auto outcome = parse_reading("R|003|S-100001|GLU|2130.5|-|2026-01-01T00:00:00Z");
  const auto* reading = std::get_if<RawReading>(&outcome);
  ASSERT_NE(reading, nullptr);
  EXPECT_EQ(reading->kind, RecordKind::Result);
  EXPECT_EQ(reading->rack_position, 3);
  EXPECT_EQ(reading->sample_id, "S-100001");
  EXPECT_EQ(reading->assay, "GLU");
  EXPECT_DOUBLE_EQ(reading->signal, 2130.5);
  EXPECT_EQ(reading->flags, 0U);
  EXPECT_EQ(reading->firmware_version, 2);
}

TEST(ResultParser, ParsesErrorRecord) {
  const auto outcome = parse_reading("E|002|S-100002|41|clot detected");
  const auto* reading = std::get_if<RawReading>(&outcome);
  ASSERT_NE(reading, nullptr);
  EXPECT_EQ(reading->kind, RecordKind::Error);
  EXPECT_EQ(reading->error_code, 41);
  EXPECT_EQ(reading->message, "clot detected");
  EXPECT_TRUE(has_flag(reading->flags, ResultFlag::InstrumentError));
}

TEST(ResultParser, SkipsHeartbeat) {
  EXPECT_TRUE(std::holds_alternative<Skipped>(parse_reading("H|alive")));
}

}  // namespace
}  // namespace lab::io
