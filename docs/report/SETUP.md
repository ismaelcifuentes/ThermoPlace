# 5. Experimental setup (draft — Jose)

> Section 5 of the final report. Everything was run with the tag `v1.0` (commit `3a1ca55`). The raw tables are in `docs/RESULTS.md` (Eimi) and `docs/VALIDATION.md` (Jose).

## 5.1 Benchmarks

We use two MCNC benchmarks with a fixed outline and three GSRC benchmarks without one (Table 5.1). Dimensions are in micrometres. A net may connect blocks and fixed I/O pads ("terminals"); pads stay at the coordinates given in the benchmark.

| Benchmark | Blocks | Pads | Nets | Total block area | Fixed outline (µm) | Free space in the outline |
|---|---|---|---|---|---|---|
| ami33 | 33 | 40 | 121 | 1.156 mm² | 1326 × 1205 | 27.6 % |
| ami49 | 49 | 22 | 396 | 35.45 mm² | 5336 × 7673 | 13.4 % |
| n100 | 100 | 334 | 885 | 0.180 mm² | none (15 % in one run) | — |
| n200 | 200 | 564 | 1585 | 0.176 mm² | none | — |
| n300 | 300 | 569 | 1893 | 0.273 mm² | none | — |

*Table 5.1. Benchmarks. Free space = 1 − block area / outline area.*

The GSRC files have no outline. For the fixed-outline run on n100 we use a square outline with 15 % free space, of side (1.15 · total block area)<sup>1/2</sup>, as is usual in the fixed-outline literature [Adya] (option `--whitespace 0.15`). The outline of ami49 is the tightest one we use: only 13.4 % of it is free.

## 5.2 Power

The benchmarks give no power, so we assign it synthetically (Section 3.6.1): each block gets a power density drawn uniformly from a range, with the random generator seeded by the *power seed*, and power = density × area. We always use **power seed 1**, so every experiment sees the same power map, and two ranges:

| Range | Use | Total power, ami33 | Total power, ami49 |
|---|---|---|---|
| 0.5–2 W/mm² | default: validation (Section 4) and the area–temperature trade-off (Section 6.2) | 1.61 W | 50.5 W |
| 0.1–10 W/mm² | strongly uneven power: sensitivity study (Section 6.3, Section 4.4) | 6.91 W | 220.1 W |

*Table 5.2. Power ranges (power seed 1).*

The power seed is separate from the SA seed: changing the SA seed changes the floorplan, not the power of each block.

## 5.3 Optimizer and thermal model settings

Unless a table says otherwise, all runs use the settings in Table 5.3.

| Setting | Value |
|---|---|
| Cost weights | α = 0.5 (area vs. wirelength), outline weight β = 5, thermal weight γ = 0 (γ = 0–1 in Section 6.2, 0–0.5 in Section 6.3) |
| SA, default | cooling factor 0.95, *M* = 10*n* moves per temperature, initial acceptance probability 0.9, stop when *T* < 10⁻⁴ *T*<sub>0</sub> (about 180 temperatures) |
| SA, "long" | cooling factor 0.98, *M* = 2000 moves per temperature |
| Feasibility | with β > 0, SA returns the best floorplan that fits the outline, if it found one |
| SA seeds | 1 to 5 for every configuration |
| Thermal model | ambient 45 °C, silicon 130 W/(m·K), die 0.15 mm thick; package *h* = 1.4·10⁵ W/(m²·K) and *R*<sub>pkg</sub> = 0.20 K/W (calibrated in Section 4.2); the die is the fixed outline |
| Thermal grid | 16 × 16 with tolerance 10⁻³ K inside SA; 32 × 32 with tolerance 10⁻⁶ K for every reported temperature |

*Table 5.3. Default settings (`v1.0`).*

As a baseline we use a **greedy random search**: starting from the initial tree, it tries 20 000 random moves (swap two blocks or rotate one) and keeps a move only if it reduces the chip area. It ignores wirelength and the outline, so it shows what SA adds over plain local search.

## 5.4 Experiments

| Experiment | What it measures | Benchmarks | Configurations | Reported in |
|---|---|---|---|---|
| E10 | Area and wirelength | ami33, ami49 | random search; SA; SA long | Section 6.1 |
| J18 | Ablation: quadratic vs. linear outline penalty | ami33, ami49, n100 (15 %) | same runs with each penalty | Section 6.1 |
| E13 | Area–temperature trade-off | ami33, ami49 | SA, γ = 0, 0.25, 0.5, 0.75, 1; 0.5–2 W/mm² | Section 6.2 |
| E14 | Power sensitivity | ami33, ami49 | SA, γ = 0, 0.25, 0.5; 0.1–10 W/mm² | Section 6.3 |
| E6 | Scalability | n100, n200, n300 | SA without outline; n100 also with a 15 % outline | Section 6.4 |
| J15, J17 | Thermal model vs. HotSpot | ami33, ami49 | grids 8–64; both power ranges; γ = 0 and 0.25 | Section 4 |

*Table 5.4. Experiments. Each configuration is run with SA seeds 1–5.*

## 5.5 Metrics

- **Dead space**: 1 − (total block area / area of the chip bounding box). For a chip that fits, the bounding box is inside the outline, so the dead space can be lower than the free space of the outline.
- **Area**: area of the chip bounding box, in mm².
- **HPWL**: half-perimeter wirelength, as defined in Section 3.4 (block centres and fixed pads, every net with weight 1), in µm.
- **Fits**: number of seeds (out of 5) whose floorplan lies inside the fixed outline.
- **T<sub>max</sub>**: the highest cell temperature of the final floorplan on the 32 × 32 grid. When we compare with HotSpot (Section 4), which gives one temperature per block, we use the block temperatures and the hottest block instead.
- **Time**: wall-clock time of one SA run, measured inside the program with `std::chrono::steady_clock`, from the start of SA to the end (reading the files and writing the outputs are not included).

Tables give the mean over the 5 seeds and, where useful, the best seed. The best dead space and the best HPWL may come from different seeds.

## 5.6 Machines and reproducibility

ThermoPlace is single-threaded C++17, built with `g++ -O2` and no external libraries. The results of Section 6 were run on a Windows 11 PC with MinGW g++ (Eimi), and the validation of Section 4 on an Intel Core i5-10210U (1.6 GHz) laptop with Ubuntu 26.04 and g++ 15.2 (Jose), with HotSpot v7.0. All random numbers come from `std::mt19937`, so the same seed gives the same floorplan on both machines; only the running times depend on the machine.

All results come from the tag `v1.0` of the repository. To repeat any of them:

```bash
git checkout v1.0
./build.sh                       # build.bat on Windows
build/thermoplace benchmarks/mcnc/ami33.block --out results --sa --outline-weight 5 --seed 1
```

The exact command of every table is next to it in `docs/RESULTS.md`, and the validation against HotSpot is repeated with `bash tools/run_j15.sh` (`tools/README.md`).

## References

- [Adya] S. N. Adya and I. L. Markov, "Fixed-outline floorplanning: enabling hierarchical design," *IEEE Trans. Very Large Scale Integration (VLSI) Systems*, vol. 11, no. 6, pp. 1120–1135, Dec. 2003.
