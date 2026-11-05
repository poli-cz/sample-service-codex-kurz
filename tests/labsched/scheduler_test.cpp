#include "labsched/scheduler.hpp"

#include <gtest/gtest.h>

#include "test_config.hpp"

namespace lab::sched {
namespace {

class SchedulerTest : public ::testing::Test {
 protected:
  io::ServiceConfig config = testing::small_config();
  SampleQueue queue{config.queue_capacity};
  SimulatedInstrument instrument{config};
  Scheduler scheduler{config, queue, instrument};

  void submit(const std::string& id, const std::string& assay = "GLU",
              Priority priority = Priority::Routine) {
    ASSERT_EQ(scheduler.submit(Sample{id, assay, priority, 0}).status, SubmitStatus::Queued);
  }
};

TEST_F(SchedulerTest, RejectsUnknownAssay) {
  const auto outcome = scheduler.submit(Sample{"S-100001", "XYZ", Priority::Routine, 0});
  EXPECT_EQ(outcome.status, SubmitStatus::Rejected);
  EXPECT_EQ(outcome.reason, "unknown assay XYZ");
}

TEST_F(SchedulerTest, LoadFillsRackFromQueueInPriorityOrder) {
  submit("S-100001");
  submit("S-100002");
  submit("S-100003", "K", Priority::Urgent);
  submit("S-100004");
  submit("S-100005");
  EXPECT_EQ(scheduler.load_rack(), 4U);
  EXPECT_EQ(queue.size(), 1U);
  EXPECT_EQ(queue.peek_next()->id, "S-100005");
  const auto slots = scheduler.rack().slots();
  EXPECT_EQ(slots.front().sample.id, "S-100003");
}

TEST_F(SchedulerTest, RunMeasuresEverySampleOnTheRack) {
  submit("S-100001");
  submit("S-100002", "K");
  const auto report = scheduler.run();
  EXPECT_TRUE(report.errors.empty());
  ASSERT_EQ(report.results.size(), 2U);
  EXPECT_EQ(report.results[0].sample_id, "S-100001");
  EXPECT_EQ(report.results[1].unit, "mmol/L");
  EXPECT_TRUE(scheduler.rack().empty());
}

TEST_F(SchedulerTest, ConcentrationMatchesInstrumentWithinNoise) {
  submit("S-100001");
  const auto report = scheduler.run();
  ASSERT_EQ(report.results.size(), 1U);
  const auto expected = instrument.true_concentration("S-100001", "GLU");
  EXPECT_NEAR(report.results[0].concentration, expected, expected * 0.05);
}

TEST_F(SchedulerTest, CancelRemovesFromQueueOrRack) {
  submit("S-100001");
  submit("S-100002");
  scheduler.load_rack();
  submit("S-100003");
  EXPECT_TRUE(scheduler.cancel("S-100003"));
  EXPECT_TRUE(scheduler.cancel("S-100001"));
  EXPECT_FALSE(scheduler.cancel("S-100001"));
  EXPECT_EQ(scheduler.rack().size(), 1U);
}

TEST_F(SchedulerTest, StatWithFreeRackPositionIsQueued) {
  submit("S-100001");
  scheduler.load_rack();
  submit("S-100009", "GLU", Priority::Stat);
  EXPECT_EQ(queue.count(Priority::Stat), 1U);
}

}  // namespace
}  // namespace lab::sched
