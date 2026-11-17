# ThermoPlace

**Thermal-aware floorplanning of chips with B\*-trees and simulated annealing (C++17).**

Advanced Programming final project, Yachay Tech.
Team: **Ismael Xavier Cifuentes Carvajal** (team lead), **Eimi Stefany Sevilla Flores**, **Jose Fernando Reinoso Hernandez**.

Repository: https://github.com/ismaelcifuentes/ThermoPlace

## Motivation

Heat is now one of the main limits in chip design, and the industry move to
chiplets (small dies placed side by side or stacked) makes hot spots worse.
A floorplanner decides where each block of a chip goes. Classic floorplanners
minimize area and wirelength only; ThermoPlace also takes temperature into
account during the optimization.

**Research question.** How simple can the thermal model inside the optimizer be
while still agreeing with a reference thermal simulator (HotSpot)? And what
does each degree of peak temperature cost in extra area?

## Approach

1. Read standard public benchmarks (MCNC ami33, ami49, apte, hp, xerox; GSRC n100–n300).
2. Represent a floorplan with a **B\*-tree** (Chang et al., DAC 2000) and pack it
   in O(n²) with a contour. The O(n) contour was dropped on 4 Nov to recover a week
   of delay (see `docs/PLAN.md`).
3. Optimize area + wirelength + fixed outline with **simulated annealing** (weeks 2–3, done).
4. Compute the steady-state temperature with a fast **grid thermal model**
   (Gauss-Seidel with over-relaxation on an N × N grid, week 4), put it into the SA
   cost and validate it against **HotSpot** using the exported `.flp` and `.ptrace`
   files (week 5, done).
5. Measure scaling on GSRC n100–n300 and write a short paper (week 6).

## Status (week 6, 17 Nov 2026)

| Component | Owner | Status |
|---|---|---|
| Data model (`Design`, `Block`, `Net`) | team | done |
| Benchmark reader: MCNC (`.block`, `.nets`) and GSRC (`.hardblocks`, `.pl`, `.nets`) | Eimi | done |
| B\*-tree + contour packing, `moveNode`, `moveSubtree` | Ismael | done |
| Metrics (area, dead space, HPWL), outline check and overlap validator | Jose | done |
| SVG picture (with fixed outline), HotSpot `.flp` and `.ptrace`, heat map | Jose | done |
| Simulated annealing (Metropolis, geometric cooling, best feasible solution) | Ismael | done |
| Cost: area, HPWL, outline penalty and peak temperature rise | Jose | done |
| Command line, trace and runs CSV, convergence plot | Eimi, Jose | done |
| Results table against published work (`docs/RESULTS.md`) | Eimi, Jose | done |
| Grid thermal model, two-part package, warm-start evaluator | Ismael | done |
| Synthetic power, thermal options (`--thermal`, `--thermal-weight`, ...) | Jose, Eimi | done |
| Validation against HotSpot and package calibration (`docs/VALIDATION.md`) | Jose | done |
| Pareto front area vs. Tmax (E13) | Eimi | done |
| Power sensitivity, 0.1 to 10 W/mm² (E14), validation with uneven power (J17) | Eimi, Jose | done |
| Method section (`docs/report/METHOD.md`), calibrated defaults, SOR factor from the conductances (I16) | Ismael | done |
| GSRC n100–n300 scalability (E6), report: Introduction, Related work | Eimi | in progress |

All 57 tests pass on `main`.

### Results so far

ami33 and ami49, seeds 1 to 5, `--alpha 0.5`. Mean over the 5 seeds; "fits" counts the
seeds whose floorplan is inside the benchmark's fixed outline. Times are from one
Windows 11 PC (MinGW g++ -O2). Full tables, best values and sources: [docs/RESULTS.md](docs/RESULTS.md).

| Benchmark | Method | Dead space | HPWL | Fits | Time (s) |
|---|---|---|---|---|---|
| ami33 | random search, 20 000 iterations | 23.27% | 146 156 | 0/5 | 0.15 |
| ami33 | SA, `--outline-weight 5` | 15.63% | 92 976 | 5/5 | 0.59 |
| ami33 | SA, `--outline-weight 5 --cooling 0.98 --moves-per-temp 2000` | 12.77% | 84 265 | 5/5 | 9.77 |
| ami49 | random search, 20 000 iterations | 26.11% | 2 107 536 | 0/5 | 0.27 |
| ami49 | SA, `--outline-weight 5` | 11.90% | 1 020 173 | 3/5 | 1.67 |
| ami49 | SA, `--outline-weight 5 --cooling 0.98 --moves-per-temp 2000` | 10.80% | 906 049 | 2/5 | 17.06 |

For reference, the original B\*-tree paper reports 1.27 mm² for ami33 and 36.80 mm² for
ami49 (Table 2), optimizing area only and without a fixed outline; our best SA runs give
1.28 mm² and 39.13 mm² while also fitting the outline. ami49 is hard to fit: its outline
leaves only 13.4% free space.

**Thermal model.** Against HotSpot v7.0 (block mode) on the best floorplans of ami33 and
ami49, the block temperatures of the 16 × 16 grid differ by 0.55 °C on average, with
one pair of package values for both benchmarks (h = 1.4e5 W/(m²·K), R = 0.20 K/W). A
16 × 16 solve takes under 0.5 ms (`docs/VALIDATION.md`). With much more uneven power
(0.1 to 10 W/mm²) the error grows to 1.9 °C on ami33 and 3.7 °C on ami49 (J17).

**Area against temperature.** With power densities of 0.5 to 2 W/mm², the thermal term
lowers the peak temperature of ami33 by 1.75 °C for 16.7 more points of dead space; with
0.1 to 10 W/mm², a weight of 0.25 lowers it by 4.33 °C for 1.9 points. Most of the
temperature rise comes from the package and does not depend on the floorplan
(`docs/RESULTS.md`, E13 and E14).

Reproduce one SA run with
`build\thermoplace.exe benchmarks\mcnc\ami33.block --sa --outline-weight 5 --seed 1`.

## Build

Requirements: a C++17 compiler (MinGW g++ from Code::Blocks or MSYS2 on Windows, g++/clang on Linux/macOS). CMake is optional.
Team setup on Windows (Git, MSYS2, Antigravity IDE): see [docs/ANTIGRAVITY.md](docs/ANTIGRAVITY.md).

```bat
REM Windows, from the repository root
build.bat
build\thermoplace.exe benchmarks\mcnc\ami33.block --sa --outline-weight 5 --seed 1
build\thermoplace_tests.exe
```

```bash
# Linux / macOS / Git Bash
./build.sh
./build/thermoplace benchmarks/mcnc/ami33.block --sa --outline-weight 5 --seed 1
./build/thermoplace_tests
```

```bash
# CMake (any platform)
cmake -S . -B build && cmake --build build
```

Run the tests **from the repository root** (they read `benchmarks/` and write to `results/`).
You can run one person's tests or one week's: `thermoplace_tests ismael | eimi | jose | core | extra | week2 | week3 | week4`.

## Command line

```
thermoplace <blocks-file> [--out DIR] [--scale S] [--seed N] [--iters N]
            [--sa] [--alpha A] [--outline-weight W] [--cooling C] [--moves-per-temp N]
            [--runs-csv FILE] [--thermal] [--power-seed N] [--grid N]
            [--thermal-weight W] [--heat-sink-h H] [--package-r R]
            [--power-min D] [--power-max D]
```

| Option | Default | Meaning |
|---|---|---|
| `--out DIR` | `results` | where every output file goes (the folder must exist) |
| `--scale S` | 1e-6 | benchmark units to meters for the `.flp` files (MCNC is in µm) |
| `--seed N` | 1 | random seed; the same seed gives the same floorplan on any PC |
| `--iters N` | 0 | random search with N iterations (ignored with `--sa`) |
| `--sa` | off | simulated annealing instead of random search |
| `--alpha A` | 0.5 | weight of area against HPWL in the cost (0 to 1) |
| `--outline-weight W` | 0 | penalty for leaving the fixed outline; above 0, SA returns the best floorplan that fits, if it found one |
| `--cooling C` | 0.95 | temperature factor per SA step (0 to 1) |
| `--moves-per-temp N` | 0 | moves per SA temperature; 0 means 10 × number of blocks |
| `--runs-csv FILE` | `<out>/runs.csv` | table where each run adds one line |
| `--thermal` | off | synthetic power (0.5 to 2 W/mm²) and peak temperature of the initial and final floorplan |
| `--power-seed N` | 1 | seed of the synthetic power |
| `--grid N` | 32 | grid size of the reported temperature (N × N cells) |
| `--thermal-weight W` | 0 | weight of the peak temperature rise in the SA cost (needs `--thermal --sa`); SA uses a 16 × 16 grid |
| `--heat-sink-h H` | `Thermal.hpp` | die to spreader conductance, W/(m²·K) |
| `--package-r R` | `Thermal.hpp` | lumped resistance of spreader, sink and air, K/W |
| `--power-min D`, `--power-max D` | 0.5, 2 | range of the synthetic power density, W/mm² |

## Output

For each run, `results/` gets:
- `<benchmark>_initial.svg` / `<benchmark>_best.svg`: pictures of the floorplan, with the fixed outline in red,
- `<benchmark>_initial.flp` / `<benchmark>_best.flp`: HotSpot floorplan files (meters),
- `runs.csv`: one line per run (benchmark, method, seed, area, dead space, HPWL, fits outline, seconds, peak temperature).
  Non-default settings go into the method name: `sa_c0.98_m2000`, `sa_t0.5`, `sa_t0.5_p0.1-10`.

With `--sa` it also writes:
- `<benchmark>_trace.csv`: cost and acceptance rate at each temperature step,
- `<benchmark>_convergence.svg`: plot of the current and best cost.

With `--thermal` it also writes:
- `<benchmark>_best.ptrace`: block powers for HotSpot,
- `<benchmark>_best.csv`: position, size and power of each block,
- `<benchmark>_best_temps.csv`: temperature of each block,
- `<benchmark>_thermal.svg`: heat map of the final floorplan.

`runs.csv` files written before week 4 have one column less (`max_temp_c`); start a new one.

## Repository layout

```
include/thermoplace/   public interfaces (the contract between modules)
src/                   implementations (one owner per file, see AGENTS.md)
app/main.cpp           command line program
tests/                 unit and integration tests
benchmarks/            MCNC and GSRC benchmark files (see benchmarks/README.md)
docs/                  per-person manuals, plan, weekly tasks (SEMANA*.md), RESULTS.md
results/               generated output (ignored by git)
```

## References

- Y.-C. Chang, Y.-W. Chang, G.-M. Wu, S.-W. Wu. *B\*-Trees: A New Representation for Non-Slicing Floorplans.* DAC 2000, pp. 458–463.
- T.-C. Chen, Y.-W. Chang. *Modern Floorplanning Based on B\*-Tree and Fast Simulated Annealing.* IEEE TCAD, vol. 25, no. 4, pp. 637–650, 2006.
- K. Skadron et al. *HotSpot* thermal model, v7: https://github.com/uvahotspot/HotSpot
- Recent chiplet thermal floorplanning: RLPlanner (arXiv 2312.16895), ATPlace2.5D (ICCAD 2024), STAMP-2.5D (arXiv 2504.21140).
