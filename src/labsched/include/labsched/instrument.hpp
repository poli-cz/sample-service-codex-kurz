#pragma once

#include <cstdint>
#include <map>
#include <string>

#include "labcore/calibration.hpp"
#include "labio/config.hpp"

namespace lab::sched {

// Deterministic stand-in for the analyzer. For a given seed and sample id it
// always produces the same result line in the firmware v2 format, so tests and
// demos are reproducible.
class SimulatedInstrument {
 public:
  explicit SimulatedInstrument(const io::ServiceConfig& config);

  // Returns one firmware line: "R|..." for a result or "E|..." for an error.
  [[nodiscard]] std::string measure(int rack_position, const std::string& sample_id,
                                    const std::string& assay) const;

  // The concentration the simulator pretends the sample really has.
  [[nodiscard]] double true_concentration(const std::string& sample_id,
                                          const std::string& assay) const;

 private:
  struct AssayModel {
    CalibrationCurve curve;
    double low = 0.0;
    double high = 0.0;
  };

  std::uint32_t seed_;
  std::map<std::string, AssayModel, std::less<>> models_;
};

// Stable 64-bit FNV-1a hash, independent of the standard library implementation.
[[nodiscard]] std::uint64_t stable_hash(const std::string& text, std::uint64_t salt = 0) noexcept;

}  // namespace lab::sched
