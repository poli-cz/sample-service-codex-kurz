#pragma once

#include <cstddef>
#include <map>
#include <optional>
#include <string_view>
#include <vector>

#include "labcore/sample.hpp"

namespace lab::sched {

struct RackSlot {
  int position = 0;  // 1-based, as printed on the rack
  Sample sample;
};

// Sample rack of the analyzer. Positions are 1..capacity.
class Rack {
 public:
  explicit Rack(std::size_t capacity);

  // Loads into the lowest free position; empty optional when the rack is full.
  std::optional<int> load(Sample sample);

  // Loads into a given free position. Returns false when the position is taken or invalid.
  bool load_at(int position, Sample sample);

  // Removes the sample at a position. Returns false when the position is empty.
  bool unload(int position);

  // Empties the rack, slots in position order.
  std::vector<RackSlot> unload_all();

  [[nodiscard]] RackSlot* find(std::string_view sample_id);

  // The slot that should give way to a STAT sample: lowest priority first,
  // among equal priorities the one submitted last. Precondition: !empty().
  [[nodiscard]] RackSlot& lowest_priority_slot();

  [[nodiscard]] std::vector<RackSlot> slots() const;
  [[nodiscard]] std::size_t size() const noexcept { return slots_.size(); }
  [[nodiscard]] std::size_t capacity() const noexcept { return capacity_; }
  [[nodiscard]] bool empty() const noexcept { return slots_.empty(); }
  [[nodiscard]] bool full() const noexcept { return slots_.size() >= capacity_; }

 private:
  std::size_t capacity_;
  std::map<int, RackSlot> slots_;
};

}  // namespace lab::sched
