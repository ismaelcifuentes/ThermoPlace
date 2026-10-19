// ThermoPlace - simulated annealing.
// OWNER: Ismael. Follow docs/SEMANA2.md. Contract: include/thermoplace/Annealer.hpp
#include "thermoplace/Annealer.hpp"

#include <cmath>
#include <random>

namespace tp {

namespace {

// What a move changed, so a rejected move can be undone.
struct Move {
    int kind = -1;      // 0 = rotate, 1 = swap, 2 = moveNode
    int rotated = -1;   // block index that was rotated (kind 0)
};

// Picks and applies one random move to `tree` (rotation lives in `d`).
// Returns the move, or kind = -1 if nothing could be changed.
Move randomMove(Design& d, BStarTree& tree, std::mt19937& rng,
                std::discrete_distribution<int>& pickKind) {
    const int n = tree.size();
    std::uniform_int_distribution<int> anyNode(0, n - 1);
    std::bernoulli_distribution coin(0.5);
    Move mv;
    mv.kind = pickKind(rng);
    if (mv.kind == 0) {                       // rotate one block 90 degrees
        mv.rotated = tree.node(anyNode(rng)).block;
        d.blocks[mv.rotated].rotated = !d.blocks[mv.rotated].rotated;
    } else if (mv.kind == 1) {                // swap the blocks of two different nodes
        int a = anyNode(rng), b = anyNode(rng);
        if (a == b) b = (a + 1) % n;
        tree.swapBlocks(a, b);
    } else {                                  // move a leaf to a free slot (a few tries)
        bool done = false;
        for (int tries = 0; tries < 20 && !done; ++tries) {
            done = tree.moveNode(anyNode(rng), anyNode(rng), coin(rng));
        }
        if (!done) mv.kind = -1;
    }
    return mv;
}

}  // namespace

// I8: simulated annealing with Metropolis acceptance and geometric cooling.
SAResult anneal(Design& d, BStarTree& tree, const CostFunction& cost, const SAParams& params) {
    SAResult res;
    const int n = tree.size();
    tree.pack(d);
    double current = cost(d);
    res.initialCost = current;
    res.bestCost = current;
    if (n < 2) return res;

    std::mt19937 rng(params.seed);
    std::uniform_real_distribution<double> unit(0.0, 1.0);
    std::discrete_distribution<int> pickKind({params.wRotate, params.wSwap, params.wMove});

    // Best solution = a copy of the tree + the rotation flags (they live in the Design).
    BStarTree bestTree = tree;
    std::vector<bool> bestRot(d.blocks.size());
    for (std::size_t i = 0; i < d.blocks.size(); ++i) bestRot[i] = d.blocks[i].rotated;

    // 1) Initial temperature: try some random moves, average the uphill deltas,
    //    and choose T0 so that an average uphill move is accepted with p0.
    double uphillSum = 0.0;
    int uphillCount = 0;
    for (int i = 0; i < 2 * n; ++i) {
        BStarTree trial = tree;
        Move mv = randomMove(d, trial, rng, pickKind);
        if (mv.kind < 0) continue;
        trial.pack(d);
        double delta = cost(d) - current;
        if (delta > 0) { uphillSum += delta; ++uphillCount; }
        if (mv.rotated >= 0) d.blocks[mv.rotated].rotated = !d.blocks[mv.rotated].rotated;
    }
    double avgUphill = (uphillCount > 0) ? uphillSum / uphillCount : 1e-6 * std::fabs(current) + 1e-9;
    const double t0 = -avgUphill / std::log(params.initialAcceptProb);
    const int movesPerTemp = (params.movesPerTemp > 0) ? params.movesPerTemp : 10 * n;

    // 2) Main loop: at each temperature try movesPerTemp moves, then cool down.
    double temp = t0;
    for (int step = 0; step < params.maxTemps && temp >= params.minTempRatio * t0; ++step) {
        int acceptedHere = 0, triedHere = 0;
        for (int k = 0; k < movesPerTemp; ++k) {
            BStarTree candidate = tree;
            Move mv = randomMove(d, candidate, rng, pickKind);
            if (mv.kind < 0) continue;
            ++triedHere;
            candidate.pack(d);
            double c = cost(d);
            double delta = c - current;
            // Metropolis: always accept a better solution; a worse one with exp(-delta/T).
            if (delta <= 0 || unit(rng) < std::exp(-delta / temp)) {
                tree = candidate;
                current = c;
                ++acceptedHere;
                if (c < res.bestCost) {
                    res.bestCost = c;
                    bestTree = tree;
                    for (std::size_t i = 0; i < d.blocks.size(); ++i) bestRot[i] = d.blocks[i].rotated;
                }
            } else if (mv.rotated >= 0) {
                d.blocks[mv.rotated].rotated = !d.blocks[mv.rotated].rotated;  // undo rotation
            }
        }
        res.moves += triedHere;
        res.accepted += acceptedHere;
        SATracePoint pt;
        pt.step = step;
        pt.temperature = temp;
        pt.currentCost = current;
        pt.bestCost = res.bestCost;
        pt.acceptRate = (triedHere > 0) ? static_cast<double>(acceptedHere) / triedHere : 0.0;
        res.trace.push_back(pt);
        ++res.temperatures;
        temp *= params.cooling;
    }

    // 3) Return the best solution: restore its tree, rotations and positions.
    tree = bestTree;
    for (std::size_t i = 0; i < d.blocks.size(); ++i) d.blocks[i].rotated = bestRot[i];
    tree.pack(d);
    res.bestCost = cost(d);
    return res;
}

}  // namespace tp
