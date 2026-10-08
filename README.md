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
   in O(n²) with a contour (O(n) version planned).
3. Optimize area + wirelength with **simulated annealing** (week 3–4).
4. Add a fast **grid thermal model** (heat diffusion on a matrix) to the cost
   function, and validate it against **HotSpot** using the exported `.flp` files (week 5–6).
5. Benchmark area, wirelength, peak temperature, runtime and scaling; write a short paper (week 7–8).

## Status (review 1)

| Component | Owner | Status |
|---|---|---|
| Data model (`Design`, `Block`, `Net`) | team | done |
| MCNC benchmark reader (`.block`, `.nets`) | Eimi | in progress |
| B\*-tree + contour packing | Ismael | in progress |
| Metrics (area, dead space, HPWL) and overlap validator | Jose | in progress |
| SVG picture + HotSpot `.flp` export | Jose | in progress |
| Random-search demo (precursor of simulated annealing) | Eimi | in progress |
| Unit tests (31 tests, no external framework) | team | done |

Results on ami33 (fill in after integration):

| Benchmark | Blocks | Chip area | Dead space | HPWL |
|---|---|---|---|---|
| ami33 initial | 33 | | | |
| ami33 after random search | 33 | | | |

## Build

Requirements: a C++17 compiler (MinGW g++ from Code::Blocks or MSYS2 on Windows, g++/clang on Linux/macOS). CMake is optional.
Team setup on Windows (Git, MSYS2, Antigravity IDE): see [docs/ANTIGRAVITY.md](docs/ANTIGRAVITY.md).

```bat
REM Windows, from the repository root
build.bat
build\thermoplace.exe benchmarks\mcnc\ami33.block --out results --iters 20000
build\thermoplace_tests.exe
```

```bash
# Linux / macOS / Git Bash
./build.sh
./build/thermoplace benchmarks/mcnc/ami33.block --out results --iters 20000
./build/thermoplace_tests
```

```bash
# CMake (any platform)
cmake -S . -B build && cmake --build build
```

Run the tests **from the repository root** (they read `benchmarks/` and write to `results/`).
You can run one person's tests: `thermoplace_tests ismael | eimi | jose | core | extra`.

## Output

For each run, `results/` gets:
- `<benchmark>_initial.svg` / `<benchmark>_best.svg`: pictures of the floorplan,
- `<benchmark>_initial.flp` / `<benchmark>_best.flp`: HotSpot floorplan files (meters).

## Repository layout

```
include/thermoplace/   public interfaces (the contract between modules)
src/                   implementations (one owner per file, see AGENTS.md)
app/main.cpp           command line program
tests/                 unit and integration tests
benchmarks/            MCNC and GSRC benchmark files (see benchmarks/README.md)
docs/                  per-person manuals, 8-week plan, review checklist
results/               generated output (ignored by git)
```

## References

- Y.-C. Chang, Y.-W. Chang, G.-M. Wu, S.-W. Wu. *B\*-Trees: A New Representation for Non-Slicing Floorplans.* DAC 2000.
- T.-C. Chen, Y.-W. Chang. *Modern Floorplanning Based on B\*-Tree and Fast Simulated Annealing.* IEEE TCAD 2006.
- K. Skadron et al. *HotSpot* thermal model, v7: https://github.com/uvahotspot/HotSpot
- Recent chiplet thermal floorplanning: RLPlanner (arXiv 2312.16895), ATPlace2.5D (ICCAD 2024), STAMP-2.5D (arXiv 2504.21140).
