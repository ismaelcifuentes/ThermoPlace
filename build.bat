@echo off
REM ThermoPlace - build without CMake (Windows + MinGW g++, e.g. the one bundled with Code::Blocks)
REM Run from the repository root:   build.bat
REM Then:  build\thermoplace.exe benchmarks\mcnc\ami33.block --out results
REM        build\thermoplace_tests.exe            (all tests)
REM        build\thermoplace_tests.exe ismael     (only your tests: ismael / eimi / jose)

if not exist build mkdir build
set SRC=src\Parser.cpp src\Contour.cpp src\BStarTree.cpp src\Metrics.cpp src\Validator.cpp src\Exporters.cpp src\Power.cpp
set FLAGS=-std=c++17 -O2 -Wall -Wextra -Iinclude

echo Building thermoplace.exe ...
g++ %FLAGS% %SRC% app\main.cpp -o build\thermoplace.exe || goto :error

echo Building thermoplace_tests.exe ...
g++ %FLAGS% %SRC% tests\test_main.cpp tests\test_parser.cpp tests\test_bstar.cpp tests\test_outputs.cpp tests\test_integration.cpp -o build\thermoplace_tests.exe || goto :error

echo Done.
goto :eof

:error
echo BUILD FAILED
exit /b 1
