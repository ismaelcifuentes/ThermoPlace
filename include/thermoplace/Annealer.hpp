// ThermoPlace - simulated annealing over a B*-tree.
// OWNER: Ismael  (implementation goes in src/Annealer.cpp)
//
// Simulated annealing (SA) explores many floorplans. At every step it changes
// the tree a little (a "move"), packs it and measures the cost:
//   * a better floorplan is always accepted,
//   * a worse one is accepted with probability exp(-delta / T) (Metropolis),
//     so early on (high T) it can escape local minima, which random search cannot.
// T starts high and is multiplied by `cooling` after `movesPerTemp` moves.
//
// The cost is a function passed in by the caller (Jose's computeCost in the
// program, a simple lambda in the tests), so this module does not depend on it.
//
// References: Kirkpatrick et al., Science 1983; Chen and Chang, IEEE TCAD 2006.
#pragma once

#include <functional>
#include <vector>

#include "thermoplace/BStarTree.hpp"
#include "thermoplace/Design.hpp"

namespace tp {

// Cost of a design that is ALREADY packed (lower is better).
using CostFunction = std::function<double(const Design&)>;

struct SAParams {
    // Initial temperature T0 is chosen so that an average uphill move is
    // accepted with this probability at the start: T0 = -avgUphill / ln(p).
    double initialAcceptProb = 0.9;
    double cooling = 0.95;        // T <- cooling * T after each temperature step
    int movesPerTemp = 0;         // 0 = 10 * number of blocks
    double minTempRatio = 1e-4;   // stop when T < minTempRatio * T0
    int maxTemps = 2000;          // safety stop (number of temperature steps)
    unsigned seed = 1;            // same seed -> same result
    // Mix of moves (relative weights, they do not need to add up to 1).
    double wRotate = 1.0;         // rotate one block 90 degrees
    double wSwap = 1.0;           // swapBlocks
    double wMove = 1.0;           // moveNode / moveSubtree (to a free slot)
    // WEEK 3 (optional): tells whether a packed design is acceptable, e.g. "fits
    // the fixed outline". When set, anneal returns the best ACCEPTABLE solution it
    // saw (if it saw any), even if an unacceptable one had a lower cost.
    std::function<bool(const Design&)> feasible;
};

// One row per temperature step (Eimi writes these to CSV, Jose plots them).
struct SATracePoint {
    int step = 0;              // temperature step 0, 1, 2, ...
    double temperature = 0.0;
    double currentCost = 0.0;  // cost of the current solution at the end of the step
    double bestCost = 0.0;     // best cost seen so far
    double acceptRate = 0.0;   // accepted moves / moves in this step (0..1)
};

struct SAResult {
    double initialCost = 0.0;  // cost of the tree that was passed in
    double bestCost = 0.0;     // cost of the returned tree
    long moves = 0;            // moves tried
    long accepted = 0;         // moves accepted
    int temperatures = 0;      // temperature steps done
    bool feasibleFound = false; // only meaningful when params.feasible is set
    std::vector<SATracePoint> trace;
};

// Runs simulated annealing.
// Precondition: tree.size() == d.blocks.size() and tree.isValid().
// On return: `tree` is the BEST tree found, and `d` holds the packing of that
// tree (positions AND rotation flags), so computeMetrics(d) gives its numbers
// and cost(d) == result.bestCost.
// With params.feasible set and some feasible solution found, "best" means the
// best FEASIBLE one, so result.bestCost can be higher than trace.back().bestCost.
SAResult anneal(Design& d, BStarTree& tree, const CostFunction& cost,
                const SAParams& params = SAParams());

}  // namespace tp
