#!/usr/bin/env bash
# ThermoPlace - J15: validation against HotSpot, from scratch.
# OWNER: Jose. See tools/README.md.
#
#   bash tools/run_j15.sh calibrate        floorplans, HotSpot, 2D sweep (h x R)
#   bash tools/run_j15.sh grids <h> <r>    error table per grid at those values
#
# Run it from the repository root, after ./build.sh. HotSpot must be built in
# $HOTSPOT_DIR (default ~/HotSpot), see docs/HOTSPOT.md.
set -e

HOTSPOT_DIR="${HOTSPOT_DIR:-$HOME/HotSpot}"
EXAMPLE="$HOTSPOT_DIR/examples/example1"
OUT="build/j15"
MODE="$1"

# 1. Build the helper, linked with the project sources it needs.
mkdir -p "$OUT"
g++ -std=c++17 -O2 -Iinclude tools/j15_tool.cpp src/Parser.cpp src/Power.cpp \
    src/Exporters.cpp src/Metrics.cpp src/Thermal.cpp -o "$OUT/j15_tool"

for bench in ami33 ami49; do
    for seed in 1 2 3 4 5; do
        dir="$OUT/${bench}_s$seed"
        flp="$dir/${bench}_best.flp"
        if [ "$MODE" = "calibrate" ]; then
            # 2. Best SA floorplan for this seed.
            mkdir -p "$dir"
            ./build/thermoplace "benchmarks/mcnc/$bench.block" --out "$dir" --sa \
                --outline-weight 5 --seed "$seed" --runs-csv "$OUT/runs.csv" > /dev/null
            # 3. Floorplan with fillers, then HotSpot in block mode (example1 config).
            "$OUT/j15_tool" "benchmarks/mcnc/$bench.block" "$flp" "$dir/j15" filled
            "$HOTSPOT_DIR/hotspot" -c "$EXAMPLE/example.config" \
                -materials_file "$EXAMPLE/example.materials" -model_type block \
                -f "$dir/j15_filled.flp" -p "$dir/j15_filled.ptrace" \
                -steady_file "$dir/j15_filled.steady" > "$dir/hotspot.log"
            # 4. Our model for every (heatSinkH, packageR).
            "$OUT/j15_tool" "benchmarks/mcnc/$bench.block" "$flp" "$dir/j15" sweep
        elif [ "$MODE" = "grids" ]; then
            "$OUT/j15_tool" "benchmarks/mcnc/$bench.block" "$flp" "$dir/j15" grids "$2" "$3"
        else
            echo "usage: bash tools/run_j15.sh calibrate | grids <h> <r>"
            exit 1
        fi
    done
done

# 5. Compare with HotSpot.
python3 -I tools/j15_compare.py "$MODE" "$OUT"
