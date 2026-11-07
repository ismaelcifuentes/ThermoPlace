// ThermoPlace - fast steady-state thermal model on a grid.
// OWNER: Ismael  (implementation goes in src/Thermal.cpp)
//
// The chip (its bounding box) is cut into an N x N grid of cells. Each cell
// gets the power of the blocks that cover it, and heat can flow:
//   * sideways to the 4 neighbour cells, through the silicon (conductance gx, gy),
//   * down to the ambient, through the package and heat sink (conductance gv).
// At steady state the heat that enters a cell equals the heat that leaves it:
//
//   P + gv*(Tamb - T) + gx*(T_left + T_right - 2T) + gy*(T_down + T_up - 2T) = 0
//
// which gives T as a weighted average of its neighbours, plus the effect of P.
// We solve all cells together with Gauss-Seidel: sweep the grid updating each
// cell with the newest values of its neighbours until nothing changes more
// than `tolerance`. Edges of the chip are adiabatic (no neighbour = no flow).
// To converge faster we use over-relaxation (SOR): each update moves the cell
// a bit PAST the Gauss-Seidel value, T = T + omega * (T_gs - T), 1 < omega < 2.
//
// Package (week 5, I15). HotSpot showed that one per-area heat-sink value cannot
// fit dies of very different sizes: its spreader and sink are fixed and much
// larger than the die. So the heat path below the die has two parts in series:
//   * per area (die -> spreader), conductance heatSinkH * cell area, to
//   * one common node at T_pkg = Tamb + packageR * P_total (spreader, sink and
//     air, lumped; ALL the power goes through it at steady state).
// In the cell equation above, Tamb is therefore replaced by T_pkg.
// With packageR = 0 this is the original one-parameter model.
//
// This is the "simple model" of our research question; week 5 compares it with
// HotSpot (a detailed, validated simulator) and calibrates heatSinkH and
// packageR against it (docs/VALIDATION.md).
#pragma once

#include <vector>

#include "thermoplace/Design.hpp"

namespace tp {

struct ThermalParams {
    int gridN = 32;                 // cells per side
    double unitToMeters = 1e-6;     // benchmark units -> meters (MCNC is in micrometers)
    double ambientC = 45.0;         // ambient temperature (HotSpot default, in C)
    double siliconK = 130.0;        // W/(m*K): -k_chip of HotSpot v7.0 example/template configs
    double dieThickness = 0.15e-3;  // m (HotSpot default t_chip)
    // PROVISIONAL until J15 recalibrates them against HotSpot (docs/VALIDATION.md):
    double heatSinkH = 1.15e5;      // W/(m^2*K): die -> spreader, per m^2 of die
    double packageR = 0.28;         // K/W: spreader + heat sink + air, shared by the whole die
    int maxIterations = 20000;      // safety stop for Gauss-Seidel
    double omega = 0.0;             // SOR factor: 1 = plain Gauss-Seidel,
                                    // 0 = automatic, 2 / (1 + sin(pi / (gridN + 1)))
    double tolerance = 1e-6;        // stop when the largest change in a sweep is below this (K)
    // WEEK 5: the die is the FIXED OUTLINE when the benchmark has one (and the
    // chip fits in it): the empty part of the die still spreads heat. Without
    // this, a bigger bounding box always looks cooler (more heat-sink area), and
    // SA would "cool" the chip just by spreading the blocks apart.
    bool dieIsOutline = true;
};

struct ThermalMap {
    int n = 0;                  // grid is n x n
    double cellW = 0.0;         // cell size in BENCHMARK units
    double cellH = 0.0;
    std::vector<double> power;  // watts per cell, row-major, row 0 = bottom of the chip
    std::vector<double> temp;   // degrees C per cell, same layout
    double maxTemp = 0.0;
    double avgTemp = 0.0;
    double packageTemp = 0.0;   // T_pkg = Tamb + packageR * total power (C)
    int iterations = 0;         // Gauss-Seidel sweeps done
    bool converged = false;     // true if it stopped because of `tolerance`

    double tempAt(int row, int col) const { return temp[row * n + col]; }
    double powerAt(int row, int col) const { return power[row * n + col]; }
};

// Spreads Block::power over an n x n grid that covers the die: the chip bounding
// box (0..chipW, 0..chipH), enlarged to the fixed outline when dieIsOutline is
// true and the design has one. A block puts into each cell a share of its power equal
// to (overlap area / block area), so the grid always adds up to totalPower(d).
// cellW and cellH receive the cell size in benchmark units.
std::vector<double> powerGrid(const Design& d, int n, double& cellW, double& cellH,
                              bool dieIsOutline = true);

// Builds the power grid and solves the steady-state temperatures.
// The design must be packed (positions computed) and have Block::power set.
ThermalMap solveThermal(const Design& d, const ThermalParams& p = ThermalParams());

// Same, but Gauss-Seidel starts from `initialTemp` (n*n values, row-major)
// instead of the ambient temperature. If the size does not match, it is ignored.
ThermalMap solveThermal(const Design& d, const ThermalParams& p,
                        const std::vector<double>& initialTemp);

// WEEK 5: average temperature of each block (one value per Design::blocks entry,
// same order), weighting every cell by the area the block covers in it. HotSpot
// in "block mode" reports one temperature per block, so this is what we compare.
std::vector<double> blockTemperatures(const Design& d, const ThermalMap& map);

// WEEK 5: peak temperature for simulated annealing, where it is computed for
// every move. It reuses the previous temperature map as the starting point
// (warm start): two consecutive SA floorplans are almost the same, so the
// solver needs far fewer sweeps than starting from the ambient every time.
class ThermalEvaluator {
public:
    explicit ThermalEvaluator(const ThermalParams& p = ThermalParams()) : params_(p) {}

    // Peak temperature (C) of a packed design with Block::power set.
    double maxTemp(const Design& d);

    // Forget the previous map (the next call starts from the ambient).
    void reset() { last_.clear(); }

    int calls() const { return calls_; }
    long totalIterations() const { return totalIterations_; }
    const ThermalParams& params() const { return params_; }

private:
    ThermalParams params_;
    std::vector<double> last_;   // temperatures of the previous call
    int calls_ = 0;
    long totalIterations_ = 0;
};

}  // namespace tp
