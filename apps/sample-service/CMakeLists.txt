# The command processor is a library so that tests can drive it without a process.
add_library(sample_service_app command_processor.cpp)
target_include_directories(sample_service_app PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(sample_service_app PUBLIC lab::sched lab::io lab::core fmt::fmt)
sample_set_warnings(sample_service_app)
sample_enable_tidy(sample_service_app)

add_executable(sample-service main.cpp)
target_link_libraries(sample-service PRIVATE sample_service_app)
target_compile_definitions(sample-service PRIVATE SAMPLE_SERVICE_VERSION="${PROJECT_VERSION}")
sample_set_warnings(sample-service)
sample_enable_tidy(sample-service)
