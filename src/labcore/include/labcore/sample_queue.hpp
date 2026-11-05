#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "labcore/sample.hpp"

namespace lab {

enum class SubmitResult : std::uint8_t { Accepted, Duplicate, QueueFull, InvalidId };

[[nodiscard]] std::string_view to_string(SubmitResult result) noexcept;

// Priority queue of samples waiting for the analyzer.
// STAT before URGENT before ROUTINE, first in first out within one priority.
class SampleQueue {
 public:
  explicit SampleQueue(std::size_t capacity);

  // Assigns the sample a sequence number when accepted.
  SubmitResult submit(Sample sample);

  // Puts a sample back at the head of its priority lane, e.g. after it was
  // unloaded from the rack. Ignores the capacity limit on purpose: the sample
  // was already accepted once and must not be lost.
  void requeue_front(Sample sample);

  // Removes a waiting sample. Returns false when the id is not waiting.
  bool cancel(std::string_view sample_id);

  [[nodiscard]] std::optional<Sample> pop_next();
  [[nodiscard]] const Sample* peek_next() const;

  [[nodiscard]] bool contains(std::string_view sample_id) const;
  [[nodiscard]] std::size_t size() const noexcept;
  [[nodiscard]] bool empty() const noexcept { return size() == 0; }
  [[nodiscard]] std::size_t capacity() const noexcept { return capacity_; }
  [[nodiscard]] std::size_t count(Priority priority) const noexcept;

  // Waiting samples in the order they would be served.
  [[nodiscard]] std::vector<Sample> snapshot() const;

 private:
  using Lane = std::deque<Sample>;
  static constexpr std::size_t kLanes = 3;

  [[nodiscard]] Lane& lane(Priority priority) noexcept;
  [[nodiscard]] const Lane& lane(Priority priority) const noexcept;

  std::array<Lane, kLanes> lanes_;
  std::size_t capacity_;
  std::uint64_t next_sequence_ = 1;
};

}  // namespace lab
