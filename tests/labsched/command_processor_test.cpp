#include "command_processor.hpp"

#include <gtest/gtest.h>

#include <nlohmann/json.hpp>
#include <sstream>
#include <string>
#include <vector>

#include "test_config.hpp"

namespace lab::app {
namespace {

std::vector<nlohmann::json> events(const std::string& text) {
  std::vector<nlohmann::json> out;
  std::istringstream in(text);
  std::string line;
  while (std::getline(in, line)) {
    out.push_back(nlohmann::json::parse(line));
  }
  return out;
}

TEST(CommandProcessor, SubmitAndRunEmitResults) {
  std::ostringstream out;
  CommandProcessor processor(sched::testing::small_config(), out);
  EXPECT_TRUE(processor.handle("SUBMIT S-100001 GLU"));
  EXPECT_TRUE(processor.handle("RUN"));
  EXPECT_FALSE(processor.handle("QUIT"));

  const auto e = events(out.str());
  ASSERT_EQ(e.size(), 4U);
  EXPECT_EQ(e[0]["status"], "queued");
  EXPECT_EQ(e[1]["event"], "result");
  EXPECT_EQ(e[1]["sample_id"], "S-100001");
  EXPECT_EQ(e[2]["event"], "cycle_done");
  EXPECT_EQ(e[3]["event"], "bye");
  EXPECT_EQ(processor.results().size(), 1U);
}

TEST(CommandProcessor, IgnoresCommentsAndReportsUnknownCommands) {
  std::ostringstream out;
  CommandProcessor processor(sched::testing::small_config(), out);
  processor.handle("# comment");
  processor.handle("   ");
  processor.handle("FROBNICATE");
  const auto e = events(out.str());
  ASSERT_EQ(e.size(), 1U);
  EXPECT_EQ(e[0]["event"], "error");
}

TEST(CommandProcessor, RejectsUnknownPriority) {
  std::ostringstream out;
  CommandProcessor processor(sched::testing::small_config(), out);
  processor.handle("SUBMIT S-100001 GLU ASAP");
  const auto e = events(out.str());
  ASSERT_EQ(e.size(), 1U);
  EXPECT_EQ(e[0]["message"], "unknown priority ASAP");
}

}  // namespace
}  // namespace lab::app
