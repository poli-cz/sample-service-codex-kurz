add_library(labcore
  src/sample.cpp
  src/measurement.cpp
  src/calibration.cpp
  src/statistics.cpp
  src/sample_queue.cpp
  src/qc_rules.cpp)
add_library(lab::core ALIAS labcore)

target_include_directories(labcore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(labcore PUBLIC cxx_std_20)
sample_set_warnings(labcore)
sample_enable_tidy(labcore)
