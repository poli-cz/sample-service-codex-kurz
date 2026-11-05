#include <fmt/format.h>

#include <cstdlib>
#include <exception>
#include <fstream>
#include <iostream>
#include <span>
#include <string>
#include <string_view>

#include "command_processor.hpp"
#include "labio/config.hpp"

namespace {

constexpr std::string_view kUsage = R"(usage: sample-service [--config FILE] [--batch FILE]

Reads commands from FILE (or standard input) and writes one JSON object per line.
  --config FILE   service configuration (default: config/service.json)
  --batch FILE    command file instead of standard input
  --version       print the version and exit
  --help          print this help and exit
)";

struct Options {
  std::string config_path = "config/service.json";
  std::string batch_path;
};

int run(const Options& options) {
  auto config = lab::io::load_config(options.config_path);
  lab::app::CommandProcessor processor(std::move(config), std::cout);

  std::ifstream batch_file;
  if (!options.batch_path.empty()) {
    batch_file.open(options.batch_path);
    if (!batch_file) {
      std::cerr << fmt::format("cannot open batch file {}\n", options.batch_path);
      return EXIT_FAILURE;
    }
  }
  std::istream& in = options.batch_path.empty() ? std::cin : batch_file;

  std::string line;
  while (std::getline(in, line)) {
    if (!processor.handle(line)) {
      break;
    }
  }
  return EXIT_SUCCESS;
}

}  // namespace

int main(int argc, char** argv) {
  const std::span<char*> args(argv, static_cast<std::size_t>(argc));
  Options options;
  for (std::size_t i = 1; i < args.size(); ++i) {
    const std::string_view arg = args[i];
    if (arg == "--help") {
      std::cout << kUsage;
      return EXIT_SUCCESS;
    }
    if (arg == "--version") {
      std::cout << "sample-service " << SAMPLE_SERVICE_VERSION << '\n';
      return EXIT_SUCCESS;
    }
    if ((arg == "--config" || arg == "--batch") && i + 1 < args.size()) {
      (arg == "--config" ? options.config_path : options.batch_path) = args[++i];
      continue;
    }
    std::cerr << kUsage;
    return EXIT_FAILURE;
  }

  try {
    return run(options);
  } catch (const std::exception& e) {
    std::cerr << "sample-service: " << e.what() << '\n';
    return EXIT_FAILURE;
  }
}
