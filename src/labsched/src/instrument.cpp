#include "labsched/instrument.hpp"

#include <fmt/format.h>

#include <cmath>
#include <numbers>
#include <random>
#include <stdexcept>

namespace lab::sched {
namespace {

constexpr std::uint64_t kClotEvery = 97;       // about one sample in a hundred clots
constexpr std::uint64_t kHemolysisEvery = 23;  // hemolysed samples are more common
constexpr double kNoiseRelativeSd = 0.01;

// Uniform value in [0, 1) from the top 53 bits.
double unit_interval(std::uint64_t bits) {
  constexpr int kMantissaBits = 53;
  constexpr int kShift = 64 - kMantissaBits;
  return static_cast<double>(bits >> kShift) /
         static_cast<double>(std::uint64_t{1} << kMantissaBits);
}

// Box-Muller on std::mt19937_64 output, portable across standard libraries
// (std::normal_distribution is not).
double standard_normal(std::mt19937_64& rng) {
  const double u1 = std::max(unit_interval(rng()), 1e-12);
  const double u2 = unit_interval(rng());
  return std::sqrt(-2.0 * std::log(u1)) * std::cos(2.0 * std::numbers::pi * u2);
}

}  // namespace

std::uint64_t stable_hash(const std::string& text, std::uint64_t salt) noexcept {
  constexpr std::uint64_t kOffset = 14695981039346656037ULL;
  constexpr std::uint64_t kPrime = 1099511628211ULL;
  std::uint64_t hash = kOffset ^ salt;
  for (const char c : text) {
    hash ^= static_cast<unsigned char>(c);
    hash *= kPrime;
  }
  return hash;
}

SimulatedInstrument::SimulatedInstrument(const io::ServiceConfig& config)
    : seed_(config.instrument_seed) {
  for (const auto& [code, assay] : config.assays) {
    models_.emplace(code, AssayModel{.curve = CalibrationCurve::fit(assay.calibrators),
                                     .low = assay.reportable_low,
                                     .high = assay.reportable_high});
  }
}

double SimulatedInstrument::true_concentration(const std::string& sample_id,
                                               const std::string& assay) const {
  const auto it = models_.find(assay);
  if (it == models_.end()) {
    throw std::out_of_range("unknown assay: " + assay);
  }
  const auto& model = it->second;
  // Most samples fall inside the reportable range, some land below it, above
  // it or so far above that they need a dilution.
  constexpr double kBelowShare = 0.05;
  constexpr double kInRangeShare = 0.85;
  constexpr double kHighShare = 0.97;
  constexpr double kBelowFactor = 0.5;
  constexpr double kDiluteFactor = 2.0;
  constexpr double kMaxFactor = 3.0;
  const double band = unit_interval(stable_hash(sample_id + "/band/" + assay, seed_));
  const double u = unit_interval(stable_hash(sample_id + "/" + assay, seed_));
  double low = model.low;
  double high = model.high;
  if (band < kBelowShare) {
    low = model.low * kBelowFactor;
    high = model.low;
  } else if (band >= kHighShare) {
    low = model.high * kDiluteFactor;
    high = model.high * kMaxFactor;
  } else if (band >= kInRangeShare) {
    low = model.high;
    high = model.high * kDiluteFactor;
  }
  return low + (u * (high - low));
}

std::string SimulatedInstrument::measure(int rack_position, const std::string& sample_id,
                                         const std::string& assay) const {
  const auto hash = stable_hash(sample_id, seed_);
  if (hash % kClotEvery == 0) {
    return fmt::format("E|{:03}|{}|41|clot detected, aspiration aborted", rack_position,
                       sample_id);
  }

  const auto it = models_.find(assay);
  if (it == models_.end()) {
    return fmt::format("E|{:03}|{}|12|assay {} not loaded", rack_position, sample_id, assay);
  }

  std::mt19937_64 rng(hash);
  const double concentration = true_concentration(sample_id, assay);
  const double noise = 1.0 + (kNoiseRelativeSd * standard_normal(rng));
  const double signal = std::max(0.0, it->second.curve.signal_for(concentration) * noise);
  const char* flags = hash % kHemolysisEvery == 0 ? "HEM" : "-";
  return fmt::format("R|{:03}|{}|{}|{:.1f}|{}|2026-01-01T00:00:00Z", rack_position, sample_id,
                     assay, signal, flags);
}

}  // namespace lab::sched
