#pragma once

#include <map>
#include <string>
#include <string_view>
#include <vector>

#include "labcore/calibration.hpp"
#include "labcore/measurement.hpp"
#include "labcore/sample_queue.hpp"
#include "labio/config.hpp"
#include "labio/result_parser.hpp"
#include "labsched/instrument.hpp"
#include "labsched/rack.hpp"

namespace lab::sched {

enum class SubmitStatus : std::uint8_t {
  Queued,
  LoadedByPreemption,  // STAT sample took the place of a lower priority sample on the rack
  Rejected,
};

struct SubmitOutcome {
  SubmitStatus status = SubmitStatus::Rejected;
  std::string reason;  // empty unless rejected
};

struct CycleReport {
  std::vector<Measurement> results;
  std::vector<std::string> errors;  // lines that could not be parsed
};

// Moves samples from the queue onto the rack and runs the analyzer.
//
//   submit -> queue -> load_rack -> rack -> run -> measurements
//
// A STAT sample submitted while the rack is loaded and full replaces the
// lowest priority sample on the rack; that sample goes back to the head of
// its queue lane.
class Scheduler {
 public:
  Scheduler(const io::ServiceConfig& config, SampleQueue& queue,
            const SimulatedInstrument& instrument);

  SubmitOutcome submit(Sample sample);

  // Removes a sample from the queue or from the rack. False when not found.
  bool cancel(std::string_view sample_id);

  // Fills free rack positions from the queue. Returns the number of samples loaded.
  std::size_t load_rack();

  // Loads the rack if it is empty, measures every loaded sample and empties the rack.
  CycleReport run();

  [[nodiscard]] const Rack& rack() const noexcept { return rack_; }
  [[nodiscard]] const std::vector<std::string>& events() const noexcept { return events_; }

 private:
  bool preempt_for(Sample& stat);
  [[nodiscard]] Measurement to_measurement(const io::RawReading& reading) const;

  const io::ServiceConfig& config_;
  SampleQueue& queue_;
  const SimulatedInstrument& instrument_;
  Rack rack_;
  std::map<std::string, CalibrationCurve, std::less<>> curves_;
  std::vector<std::string> events_;
};

}  // namespace lab::sched
