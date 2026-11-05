#include "labsched/scheduler.hpp"

#include <fmt/format.h>

#include <utility>
#include <variant>

namespace lab::sched {

Scheduler::Scheduler(const io::ServiceConfig& config, SampleQueue& queue,
                     const SimulatedInstrument& instrument)
    : config_(config), queue_(queue), instrument_(instrument), rack_(config.rack_size) {
  for (const auto& [code, assay] : config.assays) {
    curves_.emplace(code, CalibrationCurve::fit(assay.calibrators));
  }
}

SubmitOutcome Scheduler::submit(Sample sample) {
  if (!config_.assays.contains(sample.assay)) {
    return {.status = SubmitStatus::Rejected,
            .reason = fmt::format("unknown assay {}", sample.assay)};
  }
  if (!is_valid_sample_id(sample.id)) {
    return {.status = SubmitStatus::Rejected,
            .reason = std::string(to_string(SubmitResult::InvalidId))};
  }
  if (rack_.find(sample.id) != nullptr) {
    return {.status = SubmitStatus::Rejected,
            .reason = std::string(to_string(SubmitResult::Duplicate))};
  }

  if (sample.priority == Priority::Stat && rack_.full() && !queue_.contains(sample.id)) {
    if (preempt_for(sample)) {
      return {.status = SubmitStatus::LoadedByPreemption, .reason = {}};
    }
  }

  const auto result = queue_.submit(std::move(sample));
  if (result != SubmitResult::Accepted) {
    return {.status = SubmitStatus::Rejected,
            .reason = std::string(to_string(result))};
  }
  return {.status = SubmitStatus::Queued, .reason = {}};
}

bool Scheduler::preempt_for(Sample& stat) {
  auto& victim = rack_.lowest_priority_slot();
  if (victim.sample.priority == Priority::Stat) {
    return false;  // the rack is full of STAT samples, the new one waits in the queue
  }

  const int position = victim.position;
  rack_.unload(position);
  rack_.load_at(position, std::move(stat));
  events_.push_back(fmt::format("preempted {} at position {}", victim.sample.id, position));
  queue_.requeue_front(std::move(victim.sample));
  return true;
}

bool Scheduler::cancel(std::string_view sample_id) {
  if (queue_.cancel(sample_id)) {
    return true;
  }
  if (const auto* slot = rack_.find(sample_id); slot != nullptr) {
    return rack_.unload(slot->position);
  }
  return false;
}

std::size_t Scheduler::load_rack() {
  std::size_t loaded = 0;
  while (!rack_.full()) {
    auto next = queue_.pop_next();
    if (!next) {
      break;
    }
    rack_.load(std::move(*next));
    ++loaded;
  }
  return loaded;
}

CycleReport Scheduler::run() {
  if (rack_.empty()) {
    load_rack();
  }
  CycleReport report;
  for (const auto& slot : rack_.unload_all()) {
    const auto line = instrument_.measure(slot.position, slot.sample.id, slot.sample.assay);
    const auto outcome = io::parse_reading(line);
    if (const auto* error = std::get_if<io::ParseError>(&outcome)) {
      report.errors.push_back(fmt::format("{}: {}", error->message, line));
      continue;
    }
    if (const auto* reading = std::get_if<io::RawReading>(&outcome)) {
      auto measurement = to_measurement(*reading);
      if (measurement.assay.empty()) {
        measurement.assay = slot.sample.assay;  // error records do not carry the assay
      }
      report.results.push_back(std::move(measurement));
    }
  }
  return report;
}

Measurement Scheduler::to_measurement(const io::RawReading& reading) const {
  Measurement m;
  m.sample_id = reading.sample_id;
  m.assay = reading.assay;
  m.rack_position = reading.rack_position;
  m.raw_signal = reading.signal;
  m.flags = reading.flags;

  if (reading.kind == io::RecordKind::Error) {
    add_flag(m.flags, ResultFlag::InstrumentError);
    return m;
  }

  const auto curve_it = curves_.find(reading.assay);
  const auto assay_it = config_.assays.find(reading.assay);
  if (curve_it == curves_.end() || assay_it == config_.assays.end()) {
    add_flag(m.flags, ResultFlag::InstrumentError);
    return m;
  }

  const auto& assay = assay_it->second;
  m.unit = assay.unit;
  m.concentration = curve_it->second.concentration_for(reading.signal);

  constexpr double kDilutionFactor = 2.0;
  if (m.concentration < assay.reportable_low) {
    add_flag(m.flags, ResultFlag::Low);
  } else if (m.concentration > assay.reportable_high) {
    add_flag(m.flags, ResultFlag::High);
    if (m.concentration > kDilutionFactor * assay.reportable_high) {
      add_flag(m.flags, ResultFlag::Dilute);
    }
  }
  return m;
}

}  // namespace lab::sched
