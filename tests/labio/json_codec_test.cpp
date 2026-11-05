#include "labio/json_codec.hpp"

#include <gtest/gtest.h>

namespace lab::io {
namespace {

TEST(JsonCodec, MeasurementRoundTrips) {
  Measurement m;
  m.sample_id = "S-100001";
  m.assay = "CRP";
  m.rack_position = 2;
  m.raw_signal = 4270.0;
  m.concentration = 50.25;
  m.unit = "mg/L";
  m.flags = ResultFlag::High | ResultFlag::Hemolysis;

  const auto back = measurement_from_json(to_json(m));
  EXPECT_EQ(back.sample_id, m.sample_id);
  EXPECT_EQ(back.assay, m.assay);
  EXPECT_EQ(back.rack_position, m.rack_position);
  EXPECT_DOUBLE_EQ(back.concentration, m.concentration);
  EXPECT_EQ(back.unit, m.unit);
  EXPECT_EQ(back.flags, m.flags);
}

TEST(JsonCodec, SampleRoundTrips) {
  const Sample s{"S-0042", "K", Priority::Urgent, 7};
  const auto back = sample_from_json(to_json(s));
  EXPECT_EQ(back.id, s.id);
  EXPECT_EQ(back.priority, Priority::Urgent);
  EXPECT_EQ(back.sequence, 7U);
}

}  // namespace
}  // namespace lab::io
