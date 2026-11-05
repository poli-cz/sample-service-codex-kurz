#include "labcore/sample_queue.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace lab {

std::string_view to_string(SubmitResult result) noexcept {
  switch (result) {
    case SubmitResult::Accepted:
      return "accepted";
    case SubmitResult::Duplicate:
      return "duplicate";
    case SubmitResult::QueueFull:
      return "queue_full";
    case SubmitResult::InvalidId:
      return "invalid_id";
  }
  return "unknown";
}

SampleQueue::SampleQueue(std::size_t capacity) : capacity_(capacity) {
  if (capacity_ == 0) {
    throw std::invalid_argument("queue capacity must be positive");
  }
}

SampleQueue::Lane& SampleQueue::lane(Priority priority) noexcept {
  return lanes_.at(static_cast<std::size_t>(priority));
}

const SampleQueue::Lane& SampleQueue::lane(Priority priority) const noexcept {
  return lanes_.at(static_cast<std::size_t>(priority));
}

SubmitResult SampleQueue::submit(Sample sample) {
  if (!is_valid_sample_id(sample.id)) {
    return SubmitResult::InvalidId;
  }
  if (contains(sample.id)) {
    return SubmitResult::Duplicate;
  }
  if (size() >= capacity_) {
    return SubmitResult::QueueFull;
  }
  sample.sequence = next_sequence_++;
  lane(sample.priority).push_back(std::move(sample));
  return SubmitResult::Accepted;
}

void SampleQueue::requeue_front(Sample sample) {
  lane(sample.priority).push_front(std::move(sample));
}

bool SampleQueue::cancel(std::string_view sample_id) {
  for (auto& l : lanes_) {
    const auto it = std::ranges::find(l, sample_id, &Sample::id);
    if (it != l.end()) {
      l.erase(it);
      return true;
    }
  }
  return false;
}

std::optional<Sample> SampleQueue::pop_next() {
  for (auto& l : lanes_) {
    if (!l.empty()) {
      Sample next = std::move(l.front());
      l.pop_front();
      return next;
    }
  }
  return std::nullopt;
}

const Sample* SampleQueue::peek_next() const {
  for (const auto& l : lanes_) {
    if (!l.empty()) {
      return &l.front();
    }
  }
  return nullptr;
}

bool SampleQueue::contains(std::string_view sample_id) const {
  return std::ranges::any_of(lanes_, [&](const Lane& l) {
    return std::ranges::find(l, sample_id, &Sample::id) != l.end();
  });
}

std::size_t SampleQueue::size() const noexcept {
  std::size_t total = 0;
  for (const auto& l : lanes_) {
    total += l.size();
  }
  return total;
}

std::size_t SampleQueue::count(Priority priority) const noexcept {
  return lane(priority).size();
}

std::vector<Sample> SampleQueue::snapshot() const {
  std::vector<Sample> out;
  out.reserve(size());
  for (const auto& l : lanes_) {
    out.insert(out.end(), l.begin(), l.end());
  }
  return out;
}

}  // namespace lab
