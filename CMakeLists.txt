cmake_minimum_required(VERSION 3.10)
project(ThermoPlace LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
if(MSVC)
  add_compile_options(/W4)
else()
  add_compile_options(-Wall -Wextra)
endif()

# Core library: everything except main()
add_library(thermoplace_core
  src/Parser.cpp        # Eimi
  src/Contour.cpp       # Ismael
  src/BStarTree.cpp     # Ismael
  src/Metrics.cpp       # Jose
  src/Validator.cpp     # Jose
  src/Exporters.cpp     # Jose
  src/Power.cpp         # Jose (extra)
)
target_include_directories(thermoplace_core PUBLIC include)

# Command line program
add_executable(thermoplace app/main.cpp)
target_link_libraries(thermoplace PRIVATE thermoplace_core)

# Tests
add_executable(thermoplace_tests
  tests/test_main.cpp
  tests/test_parser.cpp
  tests/test_bstar.cpp
  tests/test_outputs.cpp
  tests/test_integration.cpp
)
target_link_libraries(thermoplace_tests PRIVATE thermoplace_core)
target_compile_definitions(thermoplace_tests PRIVATE
  TP_BENCH_DIR="${CMAKE_SOURCE_DIR}/benchmarks"
  TP_OUT_DIR="${CMAKE_SOURCE_DIR}/results")

enable_testing()
add_test(NAME core_tests COMMAND thermoplace_tests core WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
