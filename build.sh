#!/usr/bin/env bash
# ThermoPlace - build without CMake (Linux / macOS / Git Bash). Run from the repo root.
set -e
mkdir -p build
SRC="src/Parser.cpp src/Contour.cpp src/BStarTree.cpp src/Metrics.cpp src/Validator.cpp src/Exporters.cpp src/Power.cpp"
FLAGS="-std=c++17 -O2 -Wall -Wextra -Iinclude"
g++ $FLAGS $SRC app/main.cpp -o build/thermoplace
g++ $FLAGS $SRC tests/test_main.cpp tests/test_parser.cpp tests/test_bstar.cpp tests/test_outputs.cpp tests/test_integration.cpp -o build/thermoplace_tests
echo "Done: build/thermoplace and build/thermoplace_tests"
