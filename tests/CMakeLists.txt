function(sample_add_test_suite name)
  add_executable(${name} ${ARGN})
  target_link_libraries(${name} PRIVATE GTest::gtest_main)
  sample_set_warnings(${name})
  gtest_discover_tests(${name} DISCOVERY_TIMEOUT 30)
endfunction()

sample_add_test_suite(labcore_tests
  labcore/sample_test.cpp
  labcore/calibration_test.cpp
  labcore/statistics_test.cpp
  labcore/sample_queue_test.cpp
  labcore/qc_rules_test.cpp)
target_link_libraries(labcore_tests PRIVATE lab::core)

sample_add_test_suite(labio_tests
  labio/result_parser_test.cpp
  labio/json_codec_test.cpp
  labio/config_test.cpp)
target_link_libraries(labio_tests PRIVATE lab::io)

sample_add_test_suite(labsched_tests
  labsched/test_config.cpp
  labsched/instrument_test.cpp
  labsched/rack_test.cpp
  labsched/scheduler_test.cpp
  labsched/command_processor_test.cpp)
target_link_libraries(labsched_tests PRIVATE sample_service_app lab::sched)

# End-to-end smoke test of the executable on the example batch.
add_test(NAME service.smoke.basic_batch
  COMMAND sample-service --config ${PROJECT_SOURCE_DIR}/config/service.json
                         --batch ${PROJECT_SOURCE_DIR}/examples/basic.batch)
set_tests_properties(service.smoke.basic_batch PROPERTIES
  PASS_REGULAR_EXPRESSION "\"event\":\"bye\""
  FAIL_REGULAR_EXPRESSION "\"event\":\"(error|parse_error)\"")

find_package(Python3 3.9 COMPONENTS Interpreter)
if(Python3_FOUND)
  add_test(NAME scripts.unittest
    COMMAND ${Python3_EXECUTABLE} -m unittest discover -s ${PROJECT_SOURCE_DIR}/scripts/tests -t ${PROJECT_SOURCE_DIR}/scripts)
endif()
