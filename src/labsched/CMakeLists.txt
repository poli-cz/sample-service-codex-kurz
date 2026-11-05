add_library(labsched
  src/instrument.cpp
  src/rack.cpp
  src/scheduler.cpp)
add_library(lab::sched ALIAS labsched)

target_include_directories(labsched PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(labsched PUBLIC lab::core lab::io PRIVATE fmt::fmt)
sample_set_warnings(labsched)
sample_enable_tidy(labsched)
