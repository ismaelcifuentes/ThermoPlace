# Introduction (draft, Eimi)

> Draft of the *Introduction* of the final report. Numbers about our own results come from `docs/RESULTS.md` and `docs/VALIDATION.md`; every other number is quoted from the cited paper.

Power density, not transistor count, now limits how much of a chip can be used at the same time. Esmaeilzadeh et al. estimated that at 22 nm "21% of a fixed-size chip must be powered off", and more than 50 % at 8 nm [1]. The heat that does get produced is not spread evenly: some blocks dissipate several times more power per mm² than others, and the hottest point of the die, not the average, sets the cooling cost and the reliability limit. Skadron et al. showed that power metrics are "poor predictors of temperature" [2]: two floorplans with the same total power can have very different peak temperatures, depending on which blocks sit next to each other.

Floorplanning is the step of physical design that fixes those neighbours. It places the blocks of a chip inside a rectangle, without overlaps, so that area and wirelength are small and, in modern flows, the result fits a fixed outline given by the level above [3]. If temperature is ignored at this stage, a hot spot can only be fixed later with more cooling, lower clock frequency, or a new floorplan. The same problem appears again, at a larger scale, when chiplets are placed on an interposer [4].

Adding temperature to a floorplanner has a cost. Simulated annealing (SA), the usual search method, evaluates tens or hundreds of thousands of floorplans per run, so the thermal model inside it must take well under a millisecond. Detailed simulators such as HotSpot [5] are too slow for that, so thermal-aware floorplanners use simpler models. How simple can that model be before it starts to give the optimizer wrong answers? That is the question this project studies.

**ThermoPlace.** We built a fixed-outline floorplanner in C++17 with no external libraries. It uses the B\*-tree representation [6] with SA, a cost function that combines area, half-perimeter wirelength (HPWL), an outline penalty and the peak temperature rise, and a steady-state grid thermal model solved with Gauss–Seidel iteration and over-relaxation. The thermal model takes the whole fixed outline as the die and has a two-part package: a per-area path to the heat spreader and one lumped resistance for the spreader, heat sink and air.

**Contributions.**

1. An open, tested implementation (57 unit and integration tests) of B\*-tree floorplanning with fixed outline, synthetic power and HotSpot export (`.flp`, `.ptrace`), reproducible from a seed on any C++17 compiler.
2. A calibration of the grid model against HotSpot v7.0 on the MCNC benchmarks ami33 and ami49. With one pair of package values for both benchmarks, the block temperatures differ from HotSpot by 0.55 °C on average with a 16 × 16 grid, which solves in under 0.5 ms. A finer grid (64 × 64) lowers that error only to 0.53 °C. With much more uneven power (0.1 to 10 W/mm²) the same calibration still fits best, but the error grows to 1.9 °C on ami33 and 3.7 °C on ami49.
3. A measurement of the area–temperature trade-off. With power densities of 0.5 to 2 W/mm², the thermal term lowers the peak temperature of ami33 by 1.75 °C at the price of 16.7 points of dead space; with 0.1 to 10 W/mm², a smaller weight lowers it by 4.33 °C for 1.9 points. Most of the temperature rise comes from the package and does not depend on where the blocks go; the floorplan only controls the part above it.
4. The runtime of the floorplanner on GSRC n100–n300 (Section 6). [COMPLETE when E6 is merged]

The rest of the report is organized as follows. Section 2 reviews floorplan representations, thermal models and thermal-aware floorplanning. Section 3 describes the method, Section 4 its validation against HotSpot, Sections 5 and 6 the experiments, and Section 7 the conclusions and limits.

## References (numbering local to this draft)

1. H. Esmaeilzadeh, E. Blem, R. St. Amant, K. Sankaralingam and D. Burger, "Dark silicon and the end of multicore scaling," in *Proc. 38th Int. Symp. Computer Architecture (ISCA)*, 2011, pp. 365–376, doi:10.1145/2000064.2000108.
2. K. Skadron, M. R. Stan, W. Huang, S. Velusamy, K. Sankaranarayanan and D. Tarjan, "Temperature-aware microarchitecture," in *Proc. 30th ISCA*, 2003, pp. 2–13, doi:10.1109/ISCA.2003.1206984.
3. S. N. Adya and I. L. Markov, "Fixed-outline floorplanning: enabling hierarchical design," *IEEE Trans. VLSI Syst.*, vol. 11, no. 6, pp. 1120–1135, Dec. 2003, doi:10.1109/TVLSI.2003.817546.
4. Y. Ma, L. Delshadtehrani, C. Demirkiran, J. L. Abellán and A. Joshi, "TAP-2.5D: A thermally-aware chiplet placement methodology for 2.5D systems," in *Proc. Design, Automation & Test in Europe (DATE)*, 2021, doi:10.23919/DATE51398.2021.9474011.
5. W. Huang, S. Ghosh, S. Velusamy, K. Sankaranarayanan, K. Skadron and M. R. Stan, "HotSpot: A compact thermal modeling methodology for early-stage VLSI design," *IEEE Trans. VLSI Syst.*, vol. 14, no. 5, pp. 501–513, May 2006.
6. Y.-C. Chang, Y.-W. Chang, G.-M. Wu and S.-W. Wu, "B\*-Trees: A new representation for non-slicing floorplans," in *Proc. ACM/IEEE Design Automation Conf. (DAC)*, 2000, pp. 458–463.
