### 6.3 Power sensitivity (draft — Jose)

> Subsection 6.3 of the final report, to be merged into `RESULTS_SECTION.md`. All numbers were run with the tag `v1.0`; the raw table is the E14 section of `docs/RESULTS.md` (Eimi) and the HotSpot check is J17 in `docs/VALIDATION.md` (Jose).

With the default power (0.5–2 W/mm², Section 6.2), a thermal weight of 0.25 lowers the peak temperature of ami33 by only 1.18 °C. Real chips usually have a much more uneven power map, with a few hot blocks next to almost idle ones, and that is where moving blocks apart should matter most. We therefore repeated the trade-off with power densities of **0.1–10 W/mm²** (6.91 W on ami33, 220 W on ami49; Table 5.2), with thermal weights γ = 0, 0.25 and 0.5 and SA seeds 1–5.

| Benchmark | γ | Mean *T*<sub>max</sub> (°C) | Best *T*<sub>max</sub> (°C) | Mean dead space | Mean HPWL | Fits | Mean time (s) |
|---|---|---|---|---|---|---|---|
| ami33 | 0 | 85.63 | 83.71 | 11.72 % | 87 244 | 5/5 | 0.56 |
| ami33 | 0.25 | **79.92** | 79.51 | 16.91 % | 89 576 | 5/5 | 12.22 |
| ami33 | 0.5 | 79.49 | 79.06 | 20.19 % | 88 900 | 5/5 | 12.19 |
| ami49 | 0 | 158.80 | 156.59 | 12.07 % | 1 042 650 | 5/5 | 1.63 |
| ami49 | 0.25 | 159.15 | 156.94 | 11.40 % | 1 046 944 | 3/5 | 8.69 |
| ami49 | 0.5 | 159.29 | 159.04 | 12.15 % | 1 007 730 | 3/5 | 8.64 |

*Table 6.x. Thermal weight with strongly uneven power (0.1–10 W/mm², `--sa --outline-weight 5 --thermal --thermal-weight γ --power-min 0.1 --power-max 10`). T*<sub>max</sub> *is the hottest cell on the 32 × 32 grid.*

**ami33: the thermal term pays off.** With γ = 0.25 the mean peak temperature drops by **5.7 °C** (85.63 → 79.92 °C), almost five times the 1.18 °C of the default power, for 5.2 points more dead space and 2.7 % more wirelength; all five floorplans still fit the outline. Most of the gain comes from the first step: γ = 0.5 lowers *T*<sub>max</sub> by only 0.4 °C more and costs another 3.3 points of dead space. Part of the temperature does not depend on the placement at all: with *R*<sub>pkg</sub> = 0.20 K/W, the package node sits at 45 + 0.20 · 6.91 = 46.4 °C whatever the floorplan. Above that node, γ = 0.25 lowers the rise from 39.3 K to 33.5 K (15 %), a similar share to the 13 % of the default power, but on a rise four times larger.

**The gain is real.** Because our model is less accurate with this power (Section 4.4), we simulated the floorplans with γ = 0 and γ = 0.25 in HotSpot (Section 4.5, Table 4.4). HotSpot confirms a lower peak temperature on all five seeds, by 3.7 °C on average; our model overestimates the gain by about 1.9 °C, but ranks every pair of floorplans the same way.

**ami49: no gain.** The thermal term does not lower the peak temperature of ami49 (158.80 °C with γ = 0, 159.15 °C with γ = 0.25), and with γ > 0 only 3 of 5 floorplans fit the outline. HotSpot agrees: with γ = 0.25 its peak temperature is 3.6 °C higher, in the same direction as our model in 4 of 5 seeds (Section 4.5). The outline of ami49 leaves only 13.4 % free space, against 27.6 % for ami33, so there is little room to move the hot blocks apart without breaking the outline. The absolute temperatures of ami49 should also be read with care: 220 W on a 41 mm² die is not a realistic chip, and with this power our model underestimates its *T*<sub>max</sub> by about 5 °C (Section 4.4).

**Cost in time.** Computing the temperature for every move makes SA about 22 times slower on ami33 (0.56 → 12.2 s) and 5 times slower on ami49 (1.63 → 8.7 s), even with the 16 × 16 grid and the warm start of Section 3.6.3.

In short, a thermal-aware cost is worth it when the power is uneven and the outline leaves room to spread the hot blocks: on ami33 it lowers the peak temperature by 3.7 °C as measured by HotSpot. With a tight outline, as in ami49, there is no room and the thermal term does not help.
