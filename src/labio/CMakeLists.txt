add_library(labio
  src/result_parser.cpp
  src/json_codec.cpp
  src/config.cpp
  src/csv_report.cpp
  legacy/legacy_result_parser.cpp)
add_library(lab::io ALIAS labio)

target_include_directories(labio
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/legacy)
target_link_libraries(labio
  PUBLIC lab::core nlohmann_json::nlohmann_json
  PRIVATE fmt::fmt)
sample_set_warnings(labio)
sample_enable_tidy(labio)

# Documented exception: the legacy parser is C-style code ported from the old
# acquisition daemon. It keeps the full warning set, but these categories are
# not errors there until the parser is rewritten.
if(MSVC)
  set_source_files_properties(legacy/legacy_result_parser.cpp PROPERTIES
    COMPILE_DEFINITIONS _CRT_SECURE_NO_WARNINGS)
else()
  set_source_files_properties(legacy/legacy_result_parser.cpp PROPERTIES
    COMPILE_OPTIONS "-Wno-old-style-cast;-Wno-conversion;-Wno-sign-conversion;$<$<CXX_COMPILER_ID:GNU>:-Wno-stringop-truncation>")
endif()
