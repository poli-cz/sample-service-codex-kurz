#pragma once

#include <cstddef>
#include <ostream>
#include <string_view>
#include <vector>

#include "labcore/measurement.hpp"
#include "labcore/sample_queue.hpp"
#include "labio/config.hpp"
#include "labsched/instrument.hpp"
#include "labsched/scheduler.hpp"

namespace lab::app {

// Line protocol of the service. One command per line, one JSON object per
// output line. See README.md for the command list.
class CommandProcessor {
 public:
  CommandProcessor(io::ServiceConfig config, std::ostream& out);

  // Returns false after QUIT.
  bool handle(std::string_view line);

  [[nodiscard]] const std::vector<Measurement>& results() const noexcept { return results_; }

 private:
  void submit(const std::vector<std::string_view>& args);
  void cancel(const std::vector<std::string_view>& args);
  void load();
  void run();
  void status();
  void report(const std::vector<std::string_view>& args, bool summary);
  void error(std::string_view message);
  void flush_events();

  io::ServiceConfig config_;
  std::ostream& out_;
  SampleQueue queue_;
  sched::SimulatedInstrument instrument_;
  sched::Scheduler scheduler_;
  std::vector<Measurement> results_;
  std::size_t printed_events_ = 0;
};

}  // namespace lab::app
