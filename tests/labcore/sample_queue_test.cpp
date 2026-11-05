#include "labcore/sample_queue.hpp"

#include <gtest/gtest.h>

#include <string>

namespace lab {
namespace {

Sample make(std::string id, Priority priority = Priority::Routine) {
  return Sample{std::move(id), "GLU", priority, 0};
}

TEST(SampleQueue, ServesStatThenUrgentThenRoutine) {
  SampleQueue q(10);
  ASSERT_EQ(q.submit(make("S-0001")), SubmitResult::Accepted);
  ASSERT_EQ(q.submit(make("S-0002", Priority::Urgent)), SubmitResult::Accepted);
  ASSERT_EQ(q.submit(make("S-0003", Priority::Stat)), SubmitResult::Accepted);

  EXPECT_EQ(q.pop_next()->id, "S-0003");
  EXPECT_EQ(q.pop_next()->id, "S-0002");
  EXPECT_EQ(q.pop_next()->id, "S-0001");
  EXPECT_FALSE(q.pop_next().has_value());
}

TEST(SampleQueue, IsFifoWithinPriority) {
  SampleQueue q(10);
  q.submit(make("S-0001"));
  q.submit(make("S-0002"));
  q.submit(make("S-0003"));
  EXPECT_EQ(q.pop_next()->id, "S-0001");
  EXPECT_EQ(q.pop_next()->id, "S-0002");
}

TEST(SampleQueue, AssignsIncreasingSequenceNumbers) {
  SampleQueue q(10);
  q.submit(make("S-0001"));
  q.submit(make("S-0002", Priority::Stat));
  const auto snapshot = q.snapshot();
  ASSERT_EQ(snapshot.size(), 2U);
  EXPECT_EQ(snapshot[0].id, "S-0002");
  EXPECT_GT(snapshot[0].sequence, snapshot[1].sequence);
}

TEST(SampleQueue, RejectsDuplicatesInvalidIdsAndOverflow) {
  SampleQueue q(2);
  EXPECT_EQ(q.submit(make("bad")), SubmitResult::InvalidId);
  EXPECT_EQ(q.submit(make("S-0001")), SubmitResult::Accepted);
  EXPECT_EQ(q.submit(make("S-0001", Priority::Stat)), SubmitResult::Duplicate);
  EXPECT_EQ(q.submit(make("S-0002")), SubmitResult::Accepted);
  EXPECT_EQ(q.submit(make("S-0003")), SubmitResult::QueueFull);
  EXPECT_EQ(q.size(), 2U);
}

TEST(SampleQueue, CancelRemovesWaitingSample) {
  SampleQueue q(10);
  q.submit(make("S-0001"));
  q.submit(make("S-0002", Priority::Urgent));
  EXPECT_TRUE(q.cancel("S-0002"));
  EXPECT_FALSE(q.cancel("S-0002"));
  EXPECT_FALSE(q.contains("S-0002"));
  EXPECT_EQ(q.count(Priority::Urgent), 0U);
  EXPECT_EQ(q.size(), 1U);
}

TEST(SampleQueue, RequeueFrontGoesAheadOfItsLaneAndIgnoresCapacity) {
  SampleQueue q(1);
  q.submit(make("S-0001"));
  auto first = q.pop_next();
  q.submit(make("S-0002"));
  q.requeue_front(std::move(*first));
  EXPECT_EQ(q.size(), 2U);
  EXPECT_EQ(q.peek_next()->id, "S-0001");
}

TEST(SampleQueue, ZeroCapacityIsRejected) {
  EXPECT_THROW(SampleQueue(0), std::invalid_argument);
}

}  // namespace
}  // namespace lab
