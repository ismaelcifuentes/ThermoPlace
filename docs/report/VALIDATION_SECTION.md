# 4. Validation against HotSpot (draft — Ismael)

> Section 4 of the final report. All numbers were run with the tag `v1.0` and come from `docs/VALIDATION.md` (Jose, J15 and J17), where the commands to repeat them are.

A fast thermal model is only useful inside the optimizer if it ranks floorplans the way a trusted simulator would. We therefore compare our grid model with HotSpot v7.0 [HotSpot], the reference simulator of most academic thermal-aware floorplanners, and ask three questions: how close the two are, how fine the grid has to be, and whether a floorplan that our model calls cooler is also cooler in HotSpot.

## 4.1 Setup

- **Floorplans.** The best floorplan of SA with outline weight 5 (no thermal term) for ami33 and ami49, seeds 1–5: 10 floorplans, all inside the fixed outline.
- **Power.** Synthetic, density uniform in 0.5–2 W/mm², power seed 1 (1.61 W on ami33, 50.5 W on ami49). Section 4.4 repeats the study with 0.1–10 W/mm².
- **HotSpot.** Version 7.0, block mode, steady state, with the package of its `example.config` (silicon conductivity 130 W/(m·K), the same value we use). HotSpot does not accept gaps in a floorplan, so the empty part of the die is filled with 0 W blocks; the die is then the same in both tools.
- **Error.** For each block, the absolute difference between our block temperature and HotSpot's. We report the mean absolute error (MAE) over the blocks of a floorplan, averaged over the 10 floorplans ("joint MAE"), the worst block, and the error in the peak temperature *T*<sub>max</sub>.

## 4.2 Package calibration

Our model has two free parameters, both in the package (Section 3.6): the per-area conductance *h* from the die to the spreader, and the lumped resistance *R*<sub>pkg</sub> of the spreader, sink and air.

Our first model had only *h*, and no single value fitted both benchmarks: the best common value left a joint MAE of 4.7 °C, and the best value for each benchmark differed by a factor of 2. The reason is that HotSpot's spreader (3 cm) and sink (6 cm) are fixed and much larger than either die (1.3 mm and 7.7 mm), so their resistance does not scale with the die area, while *h* does. Adding the lumped term *R*<sub>pkg</sub> fixed this. A grid search over *h* from 10⁴ to 10⁷ W/(m²·K) and *R*<sub>pkg</sub> from 0 to 1 K/W gives one pair for both benchmarks:

| Calibration | *h* (W/(m²·K)) | *R*<sub>pkg</sub> (K/W) | MAE ami33 (°C) | MAE ami49 (°C) | Joint MAE (°C) |
|---|---|---|---|---|---|
| Best joint pair | 1.41·10⁵ | 0.20 | 0.39 | 0.58 | 0.49 |
| **Rounded (default in `v1.0`)** | **1.4·10⁵** | **0.20** | **0.45** | **0.60** | **0.53** |
| ami33 only | 1.78·10⁵ | 0.90 | 0.13 | — | — |
| ami49 only | 1.26·10⁵ | 0.18 | — | 0.54 | — |

*Table 4.1. Package calibration against HotSpot (grid 32 × 32).*

The minimum is clear in *h*: a 10 % change raises the error little, a factor of 2 raises it to 1.2–2 °C.

## 4.3 How fine must the grid be?

| Grid *N* | Joint MAE (°C) | Worst block (°C) | Solve time ami33 / ami49 (ms) |
|---|---|---|---|
| 8 | 0.63 | 3.0 | 0.05 / 0.03 |
| **16** | **0.53** | **2.0** | **0.36 / 0.15** |
| 32 | 0.53 | 2.2 | 3.2 / 1.2 |
| 64 | 0.53 | 2.1 | 26.8 / 9.6 |

*Table 4.2. Error against HotSpot and solve time by grid size (h = 1.4·10⁵, R*<sub>pkg</sub> *= 0.20).*

A 16 × 16 grid is enough: the joint MAE is the same as with 64 × 64, while the solve is 65–75 times faster. With *N* = 16 the error in *T*<sub>max</sub> is +0.6 °C on ami33 and −0.9 °C on ami49. A HotSpot run takes 16–31 ms on ami33 and 99–177 ms on ami49. That time includes starting the program and reading files, so the comparison is approximate, but our solve is at least 40 times faster. This is what makes it possible to compute the temperature for every one of the ~60 000–90 000 moves of an SA run.

## 4.4 Strongly uneven power

We repeated the comparison with power densities of 0.1–10 W/mm² (6.91 W on ami33, 220 W on ami49) without recalibrating. Because the chips get hotter, we also report the MAE relative to the mean temperature rise of the blocks in HotSpot.

| Benchmark | Power (W/mm²) | MAE (°C) | MAE / rise | Error in *T*<sub>max</sub> (°C) |
|---|---|---|---|---|
| ami33 | 0.5–2 | 0.45 | 6.0 % | +0.6 |
| ami33 | 0.1–10 | 1.89 | 5.9 % | +2.7 |
| ami49 | 0.5–2 | 0.61 | 3.4 % | −0.9 |
| ami49 | 0.1–10 | 3.56 | 4.7 % | −5.3 |

*Table 4.3. Error against HotSpot with uneven power (N = 16).*

Recalibrating with this power gives the same pair (*h* = 1.41·10⁵, *R*<sub>pkg</sub> = 0.20), so the calibration does not depend on the power. On ami33 the error grows only because there is more power: relative to the rise it is unchanged. On ami49 the model gets worse. The worst block is off by 11.9 °C and *T*<sub>max</sub> is 5.3 °C too low, and neither a 64 × 64 grid nor a calibration for ami49 alone removes it. With strongly uneven power, our absolute temperatures should therefore be read with care.

## 4.5 Is a "cooler" floorplan really cooler?

The question that matters for the optimizer is whether the floorplans that our model prefers are also cooler in HotSpot. We generated the floorplans of SA with thermal weight 0 and 0.25 (0.1–10 W/mm², seeds 1–5) and simulated both sets in HotSpot:

| Benchmark | *T*<sub>max</sub>, our model (0 → 0.25) | *T*<sub>max</sub>, HotSpot (0 → 0.25) | Same direction |
|---|---|---|---|
| ami33 | 84.96 → 79.39 °C (**−5.6**) | 82.19 → 78.50 °C (**−3.7**) | 5 of 5 seeds |
| ami49 | 151.60 → 153.02 °C (+1.4) | 156.68 → 160.23 °C (+3.6) | 4 of 5 seeds |

*Table 4.4. Peak block temperature without and with the thermal term, in both models.*

On ami33 the gain is real: HotSpot confirms a 3.7 °C lower peak temperature in all five seeds, while our model overestimates the gain by about 1.9 °C. On ami49 the thermal term does not help in either model. The outline of ami49 leaves only 13.4 % of free space, so there is little room to move hot blocks apart. Before the linear outline penalty (Section 3.4), ami49 seemed to cool down with the thermal term. That was an artifact: those floorplans did not fit the outline, so their die was larger and therefore cooler.

## 4.6 Limits of the validation

- Two benchmarks and one power seed; the calibration was fitted and evaluated on the same 10 floorplans, without a separate test set.
- HotSpot in block mode is itself a model, not a measurement, and the calibrated values depend on its package configuration.
- The validation uses floorplans of SA without the thermal term (Sections 4.2–4.4) and, in Section 4.5, a single thermal weight.
