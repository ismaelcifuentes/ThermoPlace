// Week 5 tests: warm start, die = outline, block temperatures (Ismael),
// cost with the thermal term (Jose). Run: thermoplace_tests week5
#include <string>
#include <vector>

#include "test_framework.hpp"
#include "thermoplace/Cost.hpp"
#include "thermoplace/Thermal.hpp"

using namespace tp;

namespace {

Design twoBlocks(double leftPower, double rightPower) {
    Design d;
    d.addBlock("hot", 1000, 1000);
    d.addBlock("cold", 1000, 1000);
    d.blocks[1].x = 1000;
    d.blocks[0].power = leftPower;
    d.blocks[1].power = rightPower;
    return d;
}

}  // namespace

// ------------------------------------------------------------- Ismael (I12-I14)

TEST(thermal_sa, warm_start_needs_fewer_sweeps) {
    Design d = twoBlocks(2.0, 0.3);
    ThermalParams p;
    p.gridN = 16;
    ThermalEvaluator ev(p);
    double first = ev.maxTemp(d);
    long afterFirst = ev.totalIterations();
    d.blocks[1].power = 0.35;  // a small change, like one SA move
    double second = ev.maxTemp(d);
    long secondSweeps = ev.totalIterations() - afterFirst;
    CHECK(secondSweeps < afterFirst);
    CHECK_EQ(ev.calls(), 2);
    CHECK_NEAR(second, solveThermal(d, p).maxTemp, 1e-4);  // same answer as a cold start
    CHECK(second > first);
}

TEST(thermal_sa, die_is_the_fixed_outline) {
    Design d = twoBlocks(1.0, 1.0);
    d.outlineW = 4000;
    d.outlineH = 2000;
    double cw = 0, ch = 0;
    std::vector<double> g = powerGrid(d, 8, cw, ch, true);
    CHECK_NEAR(cw, 500, 1e-9);  // 4000 / 8
    CHECK_NEAR(ch, 250, 1e-9);  // 2000 / 8
    double total = 0.0;
    for (double x : g) total += x;
    CHECK_NEAR(total, 2.0, 1e-12);
    CHECK_NEAR(g[7 * 8 + 7], 0.0, 1e-12);  // top-right corner of the die has no block
    powerGrid(d, 8, cw, ch, false);         // bounding box only
    CHECK_NEAR(cw, 250, 1e-9);
    // Same blocks on a bigger die run cooler: the empty silicon spreads heat.
    ThermalParams box;
    box.gridN = 16;
    box.dieIsOutline = false;
    ThermalParams die = box;
    die.dieIsOutline = true;
    CHECK(solveThermal(d, die).maxTemp < solveThermal(d, box).maxTemp);
}

TEST(thermal_sa, block_temperatures) {
    Design d = twoBlocks(2.0, 0.0);
    ThermalParams p;
    p.gridN = 16;
    ThermalMap m = solveThermal(d, p);
    std::vector<double> bt = blockTemperatures(d, m);
    REQUIRE(bt.size() == 2);
    CHECK(bt[0] > bt[1]);                       // the powered block is hotter
    CHECK(bt[0] <= m.maxTemp + 1e-9);           // an average is never above the peak
    CHECK(bt[1] > p.ambientC);                  // but heat reaches the cold block
    Design flat = twoBlocks(1.0, 1.0);
    ThermalMap mf = solveThermal(flat, p);
    std::vector<double> bf = blockTemperatures(flat, mf);
    CHECK_NEAR(bf[0], bf[1], 1e-3);             // symmetric case
}

// ------------------------------------------------------------------ Jose (J14)

TEST(cost_thermal, adds_the_temperature_term) {
    Design d;
    d.addBlock("a", 4, 2);
    Metrics m;
    m.chipW = 20;
    m.chipH = 10;
    m.chipArea = 200;
    m.hpwl = 100;
    CostNorm n;
    n.area = 100;
    n.hpwl = 50;
    n.tempRise = 20;
    CostWeights w;
    w.alpha = 0.5;  // base cost = 0.5*2 + 0.5*2 = 2
    CHECK_NEAR(computeCost(m, d, w, n, 30.0), 2.0, 1e-12);  // thermal weight 0 -> no change
    w.thermal = 2.0;
    CHECK_NEAR(computeCost(m, d, w, n, 30.0), 2.0 + 2.0 * 30.0 / 20.0, 1e-12);
    CHECK_NEAR(computeCost(m, d, w, n, 30.0) - computeCost(m, d, w, n), 3.0, 1e-12);
    n.tempRise = 0.0;  // no reference -> thermal term ignored
    CHECK_NEAR(computeCost(m, d, w, n, 30.0), 2.0, 1e-12);
}
