# Method (draft — Ismael)

> Draft of the *Method* section of the final report. Everything here describes the code in `main` as of 5 November 2026. Numbers that depend on experiments (calibrated heat-sink value, final runtimes) are marked **TBD** and come from `docs/RESULTS.md` and `docs/VALIDATION.md`.

ThermoPlace has four parts that run in a loop: a **B\*-tree** that encodes a floorplan, a **packing** step that turns the tree into block coordinates, a **cost function** that scores the floorplan, and **simulated annealing (SA)**, which changes the tree and keeps or rejects each change. A **grid thermal model** adds the peak temperature to the cost. Figure 1 shows the loop.

```
 benchmark (.block, .nets)           power (synthetic, fixed seed)
          │                                   │
          ▼                                   ▼
   ┌──────────────┐  move   ┌──────────┐  x, y   ┌───────────────┐
   │ B*-tree       │ ─────► │ packing  │ ──────► │ cost:         │
   │ (SA state)    │        │ (contour)│         │ area, HPWL,   │
   └──────────────┘        └──────────┘         │ outline, Tmax │
          ▲                                      └───────┬───────┘
          │      accept / reject (Metropolis)            │
          └──────────────────────────────────────────────┘
```
*Figure 1. One SA step. (To be redrawn as a figure for the PDF.)*

## 1. Problem

A benchmark gives a set of *n* rectangular hard blocks with width *w<sub>i</sub>* and height *h<sub>i</sub>*, a set of nets (each net connects blocks and fixed I/O pads), and optionally a fixed outline *W<sub>o</sub>* × *H<sub>o</sub>*. A floorplan assigns to every block a lower-left corner (*x<sub>i</sub>*, *y<sub>i</sub>*) and an orientation (0° or 90°), with no two blocks overlapping. We look for a floorplan that is small, has short wires, fits the outline, and — in the thermal-aware version — has a low peak temperature.

We use the MCNC benchmarks ami33 (33 blocks, 121 nets) and ami49 (49 blocks), whose outlines are 1326 × 1205 and 5336 × 7673 µm, and the GSRC n100–n300 benchmarks for scalability.

## 2. B\*-tree representation and packing

We represent a floorplan with a B\*-tree [1]: a binary tree with one block per node. The tree fixes the relative position of the blocks:

- the root block is placed at (0, 0);
- the **left child** of node *v* is placed immediately to the right of *v*: *x* = *x<sub>v</sub>* + *w<sub>v</sub>*;
- the **right child** of *v* is placed on top of *v*, with the same *x*: *x* = *x<sub>v</sub>*.

The *y* coordinate of each block comes from a **contour**, the "skyline" of the blocks already placed: a new block occupying [*x*, *x* + *w*) is dropped until it touches the highest point of the contour in that interval, and then the contour is raised to *y* + *h* there. Blocks are visited in depth-first pre-order (node, whole left subtree, then right subtree), with an explicit stack instead of recursion. The order matters: a wide right child can sit on top of blocks of its parent's left subtree, so those must already be in the contour. Every tree therefore gives a legal, overlap-free floorplan compacted to the bottom-left.

The contour is a list of segments, one per placed block. A query scans the list, so it costs O(*n*) and packing costs O(*n*²). For *n* ≤ 300 this is fast enough (Section 6); the O(*n*) linked-list contour of [1] was left out of scope.

Nodes are stored in a `std::vector` and linked by indices (−1 = no link) instead of pointers. This lets SA copy a whole tree with a plain assignment, which it needs to keep the best solution.

Rotation is stored in the block (`Block::rotated`), not in the tree; packing always uses the placed width and height, so a rotated block needs no special case.

## 3. Moves

SA changes the current solution with one of three moves, chosen with equal probability:

1. **Rotate** a random block by 90°.
2. **Swap** the blocks of two random nodes (the tree shape does not change).
3. **Move** a node to a free child slot of another node. Half of the time only a leaf is moved; the other half, the node is moved **with its whole subtree**, which changes the tree shape more. A subtree move is refused if the target is inside the moved subtree, since that would create a cycle. If no legal move is found in 20 tries, the step is skipped.

All three moves keep the tree valid: one root, every node reachable exactly once, consistent parent/child links and every block in exactly one node. The test suite checks this invariant after thousands of random moves.

## 4. Cost function

All terms are normalized by the value of the initial floorplan, so that each starts near 1 and the weights really set their relative importance:

$$
C = \alpha\,\frac{A}{A_0} + (1-\alpha)\,\frac{L}{L_0} + \beta\,P_{\text{out}} + \gamma\,\frac{\Delta T}{\Delta T_0}
$$

- *A* is the area of the chip bounding box.
- *L* is the half-perimeter wirelength (HPWL): for each net, the half perimeter of the smallest box that contains the centres of its blocks and its pads, summed over all nets with at least two pins.
- *P*<sub>out</sub> penalizes overflowing the fixed outline. With *e<sub>W</sub>* = max(0, *W* − *W<sub>o</sub>*) and *e<sub>H</sub>* = max(0, *H* − *H<sub>o</sub>*), we use *P*<sub>out</sub> = (*e<sub>W</sub>* / *W<sub>o</sub>*)² + (*e<sub>H</sub>* / *H<sub>o</sub>*)², which is 0 whenever the chip fits.
- Δ*T* = *T*<sub>max</sub> − *T*<sub>amb</sub> is the peak temperature rise (Section 6).

We use α = 0.5, β = 5 for the fixed-outline experiments, and γ from 0 to 1 for the area–temperature trade-off.

## 5. Simulated annealing

We use classical SA [3] with Metropolis acceptance and geometric cooling:

- **Initial temperature.** We apply 2*n* random moves to the initial tree, average the cost increases Δ̄ of the uphill ones, and set *T*<sub>0</sub> = −Δ̄ / ln *p*<sub>0</sub>, with *p*<sub>0</sub> = 0.9. So, at the start, an average uphill move is accepted with probability 0.9.
- **Acceptance.** A move that changes the cost by Δ is accepted if Δ ≤ 0, or with probability exp(−Δ / *T*) otherwise. A rejected rotation is undone; for the other moves the candidate tree is just discarded.
- **Schedule.** At each temperature we try *M* moves (default *M* = 10*n*), then set *T* ← 0.95 *T*. SA stops when *T* < 10⁻⁴ *T*<sub>0</sub> (about 180 temperatures) or after 2000 temperatures. A "long" setting uses 0.98 and *M* = 2000.
- **Best solution.** SA keeps a copy of the best tree and the rotation of every block. When a feasibility test is given (here, "fits the outline"), it returns the best **feasible** floorplan it has seen, even if an infeasible one had a lower cost.
- **Reproducibility.** All random numbers come from one `std::mt19937` seeded by the user. Each random value is drawn in its own statement, so the same seed gives the same run on any C++ compiler.

SA records one trace point per temperature (temperature, current cost, best cost and acceptance rate), which we use for the convergence plots.

## 6. Thermal model

### 6.1 Power

The benchmarks give no power. Like other thermal-aware floorplanning work, we assign it synthetically: each block gets a power density drawn uniformly from 0.5–2 W/mm² with a fixed seed (seed 1), and power = density × area. This gives 1.61 W for ami33 and 50.5 W for ami49.

### 6.2 Grid model

The die is divided into an *N* × *N* grid of cells. When the benchmark has a fixed outline, the die is the outline (enlarged to the bounding box if the chip overflows); otherwise it is the bounding box. Taking the whole outline as the die matters. Without it, a larger bounding box always looks cooler, because it has more heat-sink area, and SA would lower the temperature just by spreading the blocks apart.

Each block spreads its power over the cells it covers, in proportion to the covered area, so the grid always adds up to the total power. Heat flows from each cell to its four neighbours through the silicon and down to the ambient through the package and heat sink. With cells of size *a* × *b* (in metres), die thickness *t* and silicon conductivity *k*, the conductances are

$$
g_x = k\,t\,\frac{b}{a},\qquad g_y = k\,t\,\frac{a}{b},\qquad g_v = h\,a\,b ,
$$

where *h* is an equivalent heat-transfer coefficient of the package and heat sink per unit die area. At steady state, the heat entering each cell equals the heat leaving it:

$$
P_{ij} + g_v\,(T_{\text{amb}} - T_{ij}) + g_x\,(T_{i,j-1} + T_{i,j+1} - 2T_{ij}) + g_y\,(T_{i-1,j} + T_{i+1,j} - 2T_{ij}) = 0 .
$$

Cells on the edge of the die have no neighbour on that side (adiabatic boundary). We use *k* = 100 W/(m·K), *t* = 0.15 mm and *T*<sub>amb</sub> = 45 °C, the HotSpot defaults [4]. The value of *h* is **calibrated against HotSpot** in Section 7 (currently 4·10⁴ W/(m²·K); **TBD**).

### 6.3 Solver

We solve the *N*² equations with Gauss–Seidel iteration with successive over-relaxation (SOR). Each sweep updates every cell with the newest values of its neighbours, moving it a factor ω past the plain Gauss–Seidel value, with ω = 2 / (1 + sin(π / (*N* + 1))). The sweeps stop when no cell changes by more than a tolerance (10⁻⁶ K for reporting, 10⁻³ K inside SA). SOR cut the number of sweeps by a factor of 6–9 in our tests: on ami33 with *N* = 32, 679 sweeps instead of 6006.

Inside SA the temperature is computed for every move. We use *N* = 16, which gave a peak temperature within 0.01 K (ami33) and 0.1 K (ami49) of *N* = 32, and a **warm start**: each solve begins from the previous temperature map instead of the ambient, since two consecutive SA floorplans are almost the same. With these settings a thermal-aware SA run takes about 6–7 s on ami33 and ami49, against 0.2–0.6 s without temperature.

For validation, we also compute the temperature of each block as the area-weighted average of the cells it covers. This matches the block-level output of HotSpot.

## 7. Validation against HotSpot

**TBD (Jose, J15).** For the best floorplans of ami33 and ami49 (seeds 1–5), we export HotSpot's floorplan (`.flp`) and power trace (`.ptrace`) files and run HotSpot in block mode. We compare its block temperatures with ours for *N* = 8, 16, 32 and 64 (mean absolute error, maximum error, error in *T*<sub>max</sub> and run time), and choose *h* to minimize the mean error.

## 8. Implementation

ThermoPlace is written in C++17 with the standard library only, and builds with MinGW g++ on Windows, with g++/clang on Linux, or with CMake. The code is split into modules with fixed interfaces (`include/thermoplace/*.hpp`), one owner per module:

| Module | Content | Owner |
|---|---|---|
| `Parser` | MCNC and GSRC readers | Eimi |
| `Contour`, `BStarTree` | Representation, packing, moves | Ismael |
| `Annealer` | Simulated annealing | Ismael |
| `Thermal` | Power grid, thermal solver, warm-start evaluator | Ismael |
| `Metrics`, `Validator`, `Cost` | Area, HPWL, overlap check, cost function | Jose |
| `Exporters`, `Power` | SVG, HotSpot `.flp`/`.ptrace`, heat map, synthetic power | Jose |
| `Report`, `app/main.cpp` | CSV reports and command-line program | Eimi |

The interfaces are checked by 56 unit and integration tests written without an external framework (**update the count before submitting**). Every floorplan reported in the results also passes an independent overlap validator.

## References

1. Y.-C. Chang, Y.-W. Chang, G.-M. Wu and S.-W. Wu, "B\*-Trees: A New Representation for Non-Slicing Floorplans," in *Proc. ACM/IEEE Design Automation Conference (DAC)*, 2000, pp. 458–463.
2. T.-C. Chen and Y.-W. Chang, "Modern Floorplanning Based on B\*-Tree and Fast Simulated Annealing," *IEEE Trans. Computer-Aided Design of Integrated Circuits and Systems*, vol. 25, no. 4, pp. 637–650, Apr. 2006.
3. S. Kirkpatrick, C. D. Gelatt and M. P. Vecchi, "Optimization by Simulated Annealing," *Science*, vol. 220, no. 4598, pp. 671–680, 1983.
4. W. Huang, S. Ghosh, S. Velusamy, K. Sankaranarayanan, K. Skadron and M. R. Stan, "HotSpot: A Compact Thermal Modeling Methodology for Early-Stage VLSI Design," *IEEE Trans. Very Large Scale Integration (VLSI) Systems*, vol. 14, no. 5, pp. 501–513, May 2006.
