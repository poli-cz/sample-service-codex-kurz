#include "command_processor.hpp"

#include <fmt/format.h>

#include <fstream>
#include <nlohmann/json.hpp>
#include <string>
#include <utility>

#include "labio/csv_report.hpp"
#include "labio/json_codec.hpp"

namespace lab::app {
namespace {

std::vector<std::string_view> split_words(std::string_view line) {
  std::vector<std::string_view> words;
  std::size_t pos = 0;
  while (pos < line.size()) {
    const auto start = line.find_first_not_of(" \t\r", pos);
    if (start == std::string_view::npos) {
      break;
    }
    const auto end = line.find_first_of(" \t\r", start);
    words.push_back(line.substr(start, end == std::string_view::npos ? end : end - start));
    pos = end == std::string_view::npos ? line.size() : end;
  }
  return words;
}

std::string_view to_string(sched::SubmitStatus status) {
  switch (status) {
    case sched::SubmitStatus::Queued:
      return "queued";
    case sched::SubmitStatus::LoadedByPreemption:
      return "loaded_by_preemption";
    case sched::SubmitStatus::Rejected:
      return "rejected";
  }
  return "unknown";
}

}  // namespace

CommandProcessor::CommandProcessor(io::ServiceConfig config, std::ostream& out)
    : config_(std::move(config)),
      out_(out),
      queue_(config_.queue_capacity),
      instrument_(config_),
      scheduler_(config_, queue_, instrument_) {}

bool CommandProcessor::handle(std::string_view line) {
  const auto words = split_words(line);
  if (words.empty() || words.front().starts_with('#')) {
    return true;
  }
  const auto command = words.front();
  if (command == "SUBMIT") {
    submit(words);
  } else if (command == "CANCEL") {
    cancel(words);
  } else if (command == "LOAD") {
    load();
  } else if (command == "RUN") {
    run();
  } else if (command == "STATUS") {
    status();
  } else if (command == "REPORT") {
    report(words, false);
  } else if (command == "SUMMARY") {
    report(words, true);
  } else if (command == "QUIT") {
    out_ << nlohmann::json{{"event", "bye"}}.dump() << '\n';
    return false;
  } else {
    error(fmt::format("unknown command {}", command));
  }
  flush_events();
  return true;
}

void CommandProcessor::submit(const std::vector<std::string_view>& args) {
  constexpr std::size_t kMinArgs = 3;
  constexpr std::size_t kMaxArgs = 4;
  if (args.size() < kMinArgs || args.size() > kMaxArgs) {
    error("usage: SUBMIT <sample-id> <assay> [STAT|URGENT|ROUTINE]");
    return;
  }
  Sample sample;
  sample.id = std::string(args[1]);
  sample.assay = std::string(args[2]);
  if (args.size() == kMaxArgs) {
    const auto priority = parse_priority(args[3]);
    if (!priority) {
      error(fmt::format("unknown priority {}", args[3]));
      return;
    }
    sample.priority = *priority;
  }
  const auto id = sample.id;
  const auto outcome = scheduler_.submit(std::move(sample));
  nlohmann::json event = {{"event", "submit"},
                          {"sample_id", id},
                          {"status", std::string(to_string(outcome.status))}};
  if (!outcome.reason.empty()) {
    event["reason"] = outcome.reason;
  }
  out_ << event.dump() << '\n';
}

void CommandProcessor::cancel(const std::vector<std::string_view>& args) {
  if (args.size() != 2) {
    error("usage: CANCEL <sample-id>");
    return;
  }
  const bool cancelled = scheduler_.cancel(args[1]);
  out_ << nlohmann::json{{"event", "cancel"},
                         {"sample_id", std::string(args[1])},
                         {"cancelled", cancelled}}
              .dump()
       << '\n';
}

void CommandProcessor::load() {
  const auto loaded = scheduler_.load_rack();
  out_ << nlohmann::json{{"event", "load"},
                         {"loaded", loaded},
                         {"on_rack", scheduler_.rack().size()}}
              .dump()
       << '\n';
}

void CommandProcessor::run() {
  const auto report = scheduler_.run();
  for (const auto& m : report.results) {
    auto event = io::to_json(m);
    event["event"] = "result";
    out_ << event.dump() << '\n';
    results_.push_back(m);
  }
  for (const auto& e : report.errors) {
    out_ << nlohmann::json{{"event", "parse_error"}, {"message", e}}.dump() << '\n';
  }
  out_ << nlohmann::json{{"event", "cycle_done"},
                         {"results", report.results.size()},
                         {"errors", report.errors.size()},
                         {"waiting", queue_.size()}}
              .dump()
       << '\n';
}

void CommandProcessor::status() {
  nlohmann::json rack = nlohmann::json::array();
  for (const auto& slot : scheduler_.rack().slots()) {
    auto entry = io::to_json(slot.sample);
    entry["position"] = slot.position;
    rack.push_back(entry);
  }
  out_ << nlohmann::json{{"event", "status"},
                         {"queue",
                          {{"STAT", queue_.count(Priority::Stat)},
                           {"URGENT", queue_.count(Priority::Urgent)},
                           {"ROUTINE", queue_.count(Priority::Routine)}}},
                         {"rack", rack},
                         {"results", results_.size()}}
              .dump()
       << '\n';
}

void CommandProcessor::report(const std::vector<std::string_view>& args, bool summary) {
  if (args.size() != 2) {
    error(summary ? "usage: SUMMARY <file.csv>" : "usage: REPORT <file.csv>");
    return;
  }
  const std::string path(args[1]);
  std::ofstream file(path);
  if (!file) {
    error(fmt::format("cannot write {}", path));
    return;
  }
  if (summary) {
    io::write_assay_summary_csv(file, results_);
  } else {
    io::write_measurements_csv(file, results_);
  }
  out_ << nlohmann::json{{"event", summary ? "summary" : "report"},
                         {"path", path},
                         {"rows", results_.size()}}
              .dump()
       << '\n';
}

void CommandProcessor::error(std::string_view message) {
  out_ << nlohmann::json{{"event", "error"}, {"message", std::string(message)}}.dump() << '\n';
}

void CommandProcessor::flush_events() {
  const auto& events = scheduler_.events();
  for (; printed_events_ < events.size(); ++printed_events_) {
    out_ << nlohmann::json{{"event", "log"}, {"message", events[printed_events_]}}.dump() << '\n';
  }
}

}  // namespace lab::app
