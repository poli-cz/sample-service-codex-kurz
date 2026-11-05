#include "labsched/rack.hpp"

#include <stdexcept>
#include <utility>

namespace lab::sched {

Rack::Rack(std::size_t capacity) : capacity_(capacity) {
  if (capacity_ == 0) {
    throw std::invalid_argument("rack capacity must be positive");
  }
}

std::optional<int> Rack::load(Sample sample) {
  if (full()) {
    return std::nullopt;
  }
  const int last = static_cast<int>(capacity_);
  for (int position = 1; position <= last; ++position) {
    if (!slots_.contains(position)) {
      slots_.emplace(position, RackSlot{.position = position, .sample = std::move(sample)});
      return position;
    }
  }
  return std::nullopt;
}

bool Rack::load_at(int position, Sample sample) {
  if (position < 1 || position > static_cast<int>(capacity_) || slots_.contains(position)) {
    return false;
  }
  slots_.emplace(position, RackSlot{.position = position, .sample = std::move(sample)});
  return true;
}

bool Rack::unload(int position) {
  return slots_.erase(position) > 0;
}

std::vector<RackSlot> Rack::unload_all() {
  std::vector<RackSlot> out;
  out.reserve(slots_.size());
  for (auto& [position, slot] : slots_) {
    out.push_back(std::move(slot));
  }
  slots_.clear();
  return out;
}

RackSlot* Rack::find(std::string_view sample_id) {
  for (auto& [position, slot] : slots_) {
    if (slot.sample.id == sample_id) {
      return &slot;
    }
  }
  return nullptr;
}

RackSlot& Rack::lowest_priority_slot() {
  if (slots_.empty()) {
    throw std::logic_error("lowest_priority_slot on an empty rack");
  }
  RackSlot* worst = &slots_.begin()->second;
  for (auto& [position, slot] : slots_) {
    if (slot.sample.priority > worst->sample.priority ||
        (slot.sample.priority == worst->sample.priority &&
         slot.sample.sequence > worst->sample.sequence)) {
      worst = &slot;
    }
  }
  return *worst;
}

std::vector<RackSlot> Rack::slots() const {
  std::vector<RackSlot> out;
  out.reserve(slots_.size());
  for (const auto& [position, slot] : slots_) {
    out.push_back(slot);
  }
  return out;
}

}  // namespace lab::sched
