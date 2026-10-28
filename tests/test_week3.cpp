// Week 3 tests: subtree moves and feasible-aware annealing (Ismael).
// Run: thermoplace_tests week3   (or bstar_subtree / anneal_feasible)
#include <random>
#include <string>

#include "test_framework.hpp"
#include "thermoplace/Annealer.hpp"
#include "thermoplace/BStarTree.hpp"

using namespace tp;

namespace {

Design randomBlocks(int n, unsigned seed) {
    std::mt19937 rng(seed);
    std::uniform_real_distribution<double> size(1.0, 20.0);
    Design d;
    for (int i = 0; i < n; ++i) d.addBlock("r" + std::to_string(i), size(rng), size(rng));
    return d;
}

double chipW(const Design& d) {
    double w = 0.0;
    for (const Block& b : d.blocks) w = (b.x + b.w() > w) ? b.x + b.w() : w;
    return w;
}

double chipH(const Design& d) {
    double h = 0.0;
    for (const Block& b : d.blocks) h = (b.y + b.h() > h) ? b.y + b.h() : h;
    return h;
}

double bboxArea(const Design& d) { return chipW(d) * chipH(d); }

SAParams quick(unsigned seed) {
    SAParams p;
    p.seed = seed;
    p.cooling = 0.9;
    p.movesPerTemp = 100;
    p.minTempRatio = 1e-3;
    return p;
}

}  // namespace

// ---------------------------------------------------------------- Ismael (I10)

TEST(bstar_subtree, moves_a_whole_subtree) {
    // Complete tree of 5: 0 -> (1, 2), 1 -> (3, 4)
    BStarTree t;
    t.buildInitial(5);
    REQUIRE(t.moveSubtree(1, 2, true));  // node 1 with 3 and 4 goes under node 2
    CHECK_EQ(t.node(0).left, -1);
    CHECK_EQ(t.node(2).left, 1);
    CHECK_EQ(t.node(1).parent, 2);
    CHECK_EQ(t.node(1).left, 3);   // its children come along
    CHECK_EQ(t.node(1).right, 4);
    CHECK(t.isValid());
}

TEST(bstar_subtree, refuses_illegal_moves) {
    BStarTree t;
    t.buildInitial(5);
    CHECK(!t.moveSubtree(0, 3, true));   // the root cannot move
    CHECK(!t.moveSubtree(1, 3, true));   // 3 is inside the subtree of 1 (cycle)
    CHECK(!t.moveSubtree(1, 1, true));   // onto itself
    CHECK(!t.moveSubtree(3, 0, true));   // slot taken (0 already has a left child)
    CHECK(t.isValid());
    CHECK(t.moveSubtree(4, 2, false));   // a leaf also works
    CHECK(t.isValid());
}

// ----------------------------------------------------------------- Ismael (I9)

TEST(anneal_feasible, returns_a_feasible_solution) {
    // "Feasible" = not wider than the initial packing. The cost (area) ignores
    // that limit, so without I9 the best area could come from a wider chip.
    Design d = randomBlocks(25, 9);
    BStarTree t;
    t.buildInitial(25);
    t.pack(d);
    const double limitW = chipW(d) * 0.8;
    SAParams p = quick(4);
    p.feasible = [limitW](const Design& x) { return chipW(x) <= limitW; };
    SAResult r = anneal(d, t, bboxArea, p);
    REQUIRE(r.feasibleFound);
    CHECK(chipW(d) <= limitW + 1e-9);
    CHECK_NEAR(bboxArea(d), r.bestCost, 1e-6);
    CHECK(t.isValid());
}

TEST(anneal_feasible, falls_back_to_best_cost) {
    Design d = randomBlocks(15, 2);
    BStarTree t;
    t.buildInitial(15);
    t.pack(d);
    SAParams p = quick(1);
    p.feasible = [](const Design&) { return false; };  // nothing is ever feasible
    SAResult r = anneal(d, t, bboxArea, p);
    CHECK(!r.feasibleFound);
    CHECK_NEAR(r.bestCost, r.trace.back().bestCost, 1e-9);
    CHECK_NEAR(bboxArea(d), r.bestCost, 1e-6);
}
