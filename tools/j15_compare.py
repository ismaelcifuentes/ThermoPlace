"""ThermoPlace - J15: compare our block temperatures with HotSpot.

OWNER: Jose. See tools/README.md.

usage:
    python3 tools/j15_compare.py calibrate <dir>
    python3 tools/j15_compare.py grids <dir>

<dir> has one folder per floorplan, named <bench>_s<seed>, with:
    j15_filled.steady   HotSpot steady-state output (kelvin)
    j15_sweep.csv       our model, grid 32, every (heatSinkH, packageR)  [calibrate]
    j15_grids.csv       our model, grids 8..64 at the chosen values      [grids]

Error of one floorplan: MAE = mean over its blocks of |T_ours - T_HotSpot|.
The joint error is the mean MAE over all floorplans of all benchmarks.
"""
import csv
import os
import sys
from collections import defaultdict


def mean(values):
    return sum(values) / len(values)


def hotspot_temps(path, names):
    """Block temperatures in C from a HotSpot .steady file (real blocks only)."""
    temps = {}
    with open(path) as f:
        for line in f:
            parts = line.split()
            if len(parts) == 2 and parts[0] in names:
                temps[parts[0]] = float(parts[1]) - 273.15
    return temps


def our_runs(path):
    """{(grid, h, r): ({block: temp_c}, seconds)} from a j15_*.csv file."""
    runs = defaultdict(lambda: [{}, 0.0])
    with open(path) as f:
        for row in csv.DictReader(f):
            key = (int(row["grid"]), float(row["heat_sink_h"]), float(row["package_r"]))
            runs[key][0][row["block"]] = float(row["temp_c"])
            runs[key][1] = float(row["seconds"])
    return runs


def compare(root, csv_name):
    """errors[key] = list of dicts, one per floorplan, with its errors."""
    errors = defaultdict(list)
    for case in sorted(os.listdir(root)):
        folder = os.path.join(root, case)
        if not os.path.isfile(os.path.join(folder, csv_name)):
            continue
        bench = case.split("_s")[0]
        runs = our_runs(os.path.join(folder, csv_name))
        names = set(next(iter(runs.values()))[0])
        hs = hotspot_temps(os.path.join(folder, "j15_filled.steady"), names)
        if len(hs) != len(names):
            sys.exit(f"{case}: HotSpot has {len(hs)} of {len(names)} blocks")
        for key, (temps, seconds) in runs.items():
            diffs = [abs(temps[n] - hs[n]) for n in names]
            errors[key].append({
                "bench": bench,
                "mae": mean(diffs),
                "max": max(diffs),
                "tmax": max(temps.values()) - max(hs.values()),
                "hs_tmax": max(hs.values()),
                "ms": seconds * 1000.0,
            })
    return errors


def calibrate(root):
    errors = compare(root, "j15_sweep.csv")
    benches = sorted({e["bench"] for rows in errors.values() for e in rows})

    def joint(key):
        return mean([e["mae"] for e in errors[key]])

    def of(key, bench):
        return mean([e["mae"] for e in errors[key] if e["bench"] == bench])

    best = min(errors, key=joint)
    print(f"joint best: heatSinkH = {best[1]:.4g}, packageR = {best[2]:.2f} K/W, "
          f"joint MAE {joint(best):.3f} C  "
          + "  ".join(f"{b} {of(best, b):.3f}" for b in benches))
    for b in benches:
        kb = min(errors, key=lambda k: of(k, b))
        print(f"{b} alone: heatSinkH = {kb[1]:.4g}, packageR = {kb[2]:.2f}, MAE {of(kb, b):.3f} C")
    # The provisional values of Thermal.hpp, for reference.
    prov = min(errors, key=lambda k: abs(k[1] - 1.15e5) / 1.15e5 + abs(k[2] - 0.28))
    print(f"near provisional (h {prov[1]:.4g}, R {prov[2]:.2f}): joint MAE {joint(prov):.3f}  "
          + "  ".join(f"{b} {of(prov, b):.3f}" for b in benches))
    # How flat is the minimum: best packageR for a few heatSinkH values.
    print("heatSinkH | best packageR | joint MAE")
    for h in sorted({k[1] for k in errors}):
        if 0.25 * best[1] <= h <= 4 * best[1]:
            kh = min((k for k in errors if k[1] == h), key=joint)
            print(f"{h:9.4g} | {kh[2]:13.2f} | {joint(kh):.3f}")


def grids(root):
    errors = compare(root, "j15_grids.csv")
    print("bench | grid | MAE (C) | max err (C) | Tmax err (C) | HotSpot Tmax (C) | time (ms)")
    for bench in sorted({e["bench"] for rows in errors.values() for e in rows}):
        for key in sorted(errors):
            rows = [e for e in errors[key] if e["bench"] == bench]
            print(f"{bench} | {key[0]:2d} | {mean([e['mae'] for e in rows]):.3f} | "
                  f"{max([e['max'] for e in rows]):.3f} | {mean([e['tmax'] for e in rows]):+.3f} | "
                  f"{mean([e['hs_tmax'] for e in rows]):.2f} | {mean([e['ms'] for e in rows]):.2f}")
    print("joint MAE per grid: " + ", ".join(
        f"{key[0]}: {mean([e['mae'] for e in errors[key]]):.3f}" for key in sorted(errors)))


if __name__ == "__main__":
    if len(sys.argv) != 3 or sys.argv[1] not in ("calibrate", "grids"):
        sys.exit(__doc__)
    calibrate(sys.argv[2]) if sys.argv[1] == "calibrate" else grids(sys.argv[2])
