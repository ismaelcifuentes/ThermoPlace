#!/usr/bin/env bash
# ThermoPlace - J15/J17: validation against HotSpot, from scratch.
# OWNER: Jose. See tools/README.md.
#
#   bash tools/run_j15.sh calibrate         floorplans, HotSpot, 2D sweep (h x R)
#   bash tools/run_j15.sh validate <h> <r>  floorplans, HotSpot, error table per grid
#   bash tools/run_j15.sh grids <h> <r>     error table per grid (after calibrate)
#
# Environment (optional):
#   J15_OUT                output folder (default build/j15)
#   POWER_MIN, POWER_MAX   power density range in W/mm^2 (default 0.5 and 2)
#   HOTSPOT_DIR            where HotSpot is built (default ~/HotSpot)
#
# Run it from the repository root, after ./build.sh (see docs/HOTSPOT.md).
set -e

HOTSPOT_DIR="${HOTSPOT_DIR:-$HOME/HotSpot}"
EXAMPLE="$HOTSPOT_DIR/examples/example1"
OUT="${J15_OUT:-build/j15}"
MODE="$1"

if [ "$MODE" != "calibrate" ] && [ "$MODE" != "validate" ] && [ "$MODE" != "grids" ]; then
    echo "usage: bash tools/run_j15.sh calibrate | validate <h> <r> | grids <h> <r>"
    exit 1
fi

# 1. Build the helper, linked with the project sources it needs.
mkdir -p "$OUT"
g++ -std=c++17 -O2 -Iinclude tools/j15_tool.cpp src/Parser.cpp src/Power.cpp \
    src/Exporters.cpp src/Metrics.cpp src/Thermal.cpp -o "$OUT/j15_tool"

# 2-3. Best SA floorplan for one seed, the floorplan with fillers, and HotSpot
#      in block mode with the example1 configuration.
prepare() {
    local bench="$1" seed="$2" dir="$3" flp="$4"
    mkdir -p "$dir"
    ./build/thermoplace "benchmarks/mcnc/$bench.block" --out "$dir" --sa \
        --outline-weight 5 --seed "$seed" --runs-csv "$OUT/runs.csv" > /dev/null
    "$OUT/j15_tool" "benchmarks/mcnc/$bench.block" "$flp" "$dir/j15" filled
    "$HOTSPOT_DIR/hotspot" -c "$EXAMPLE/example.config" \
        -materials_file "$EXAMPLE/example.materials" -model_type block \
        -f "$dir/j15_filled.flp" -p "$dir/j15_filled.ptrace" \
        -steady_file "$dir/j15_filled.steady" > "$dir/hotspot.log" 2>&1
}

for bench in ami33 ami49; do
    for seed in 1 2 3 4 5; do
        dir="$OUT/${bench}_s$seed"
        flp="$dir/${bench}_best.flp"
        if [ "$MODE" = "calibrate" ]; then
            prepare "$bench" "$seed" "$dir" "$flp"
            # 4. Our model for every (heatSinkH, packageR).
            "$OUT/j15_tool" "benchmarks/mcnc/$bench.block" "$flp" "$dir/j15" sweep
        else
            if [ "$MODE" = "validate" ]; then
                prepare "$bench" "$seed" "$dir" "$flp"
            fi
            # 4. Our model with grids 8 to 64 at the given (heatSinkH, packageR).
            "$OUT/j15_tool" "benchmarks/mcnc/$bench.block" "$flp" "$dir/j15" grids "$2" "$3"
        fi
    done
done

# 5. Compare with HotSpot.
if [ "$MODE" = "calibrate" ]; then
    python3 -I tools/j15_compare.py calibrate "$OUT"
else
    python3 -I tools/j15_compare.py grids "$OUT"
fi
