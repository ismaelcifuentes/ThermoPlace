// Week 4 tests: thermal model (Ismael), heat-map SVG (Jose).
// Run: thermoplace_tests week4   (or thermal / thermal_svg)
// Jose's J7 (power, .ptrace, CSV) is tested by the existing *_extra groups.
#include <cmath>
#include <fstream>
#include <string>

#include "test_framework.hpp"
#include "thermoplace/Exporters.hpp"
#include "thermoplace/Thermal.hpp"

using namespace tp;

namespace {

// Two 1000 x 1000 um blocks side by side; only the left one dissipates power.
Design twoBlocks(double leftPower, double rightPower) {
    Design d;
    d.addBlock("hot", 1000, 1000);
    d.addBlock("cold", 1000, 1000);
    d.blocks[0].x = 0;
    d.blocks[1].x = 1000;
    d.blocks[0].power = leftPower;
    d.blocks[1].power = rightPower;
    return d;
}

double sum(const std::vector<double>& v) {
    double s = 0.0;
    for (double x : v) s += x;
    return s;
}

}  // namespace

// ---------------------------------------------------------------- Ismael (I11)

TEST(thermal, power_grid_conserves_power) {
    Design d = twoBlocks(1.5, 0.5);
    double cw = 0, ch = 0;
    std::vector<double> g = powerGrid(d, 8, cw, ch);
    REQUIRE(g.size() == 64);
    CHECK_NEAR(cw, 250, 1e-9);   // chip is 2000 x 1000 -> 8 columns of 250
    CHECK_NEAR(ch, 125, 1e-9);
    CHECK_NEAR(sum(g), 2.0, 1e-12);
    // Columns 0-3 are under "hot" (1.5 W over 32 cells), 4-7 under "cold".
    CHECK_NEAR(g[0 * 8 + 0], 1.5 / 32, 1e-12);
    CHECK_NEAR(g[7 * 8 + 3], 1.5 / 32, 1e-12);
    CHECK_NEAR(g[3 * 8 + 4], 0.5 / 32, 1e-12);
}

TEST(thermal, power_grid_splits_partial_cells) {
    Design d;
    d.addBlock("a", 300, 100);  // with n = 2 on a 300 x 100 chip: cells of 150 x 50
    d.blocks[0].power = 4.0;
    double cw = 0, ch = 0;
    std::vector<double> g = powerGrid(d, 2, cw, ch);
    REQUIRE(g.size() == 4);
    for (double x : g) CHECK_NEAR(x, 1.0, 1e-12);
}

TEST(thermal, no_power_means_ambient) {
    Design d = twoBlocks(0.0, 0.0);
    ThermalParams p;
    p.gridN = 8;
    ThermalMap m = solveThermal(d, p);
    CHECK(m.converged);
    CHECK_NEAR(m.maxTemp, p.ambientC, 1e-9);
    CHECK_NEAR(m.avgTemp, p.ambientC, 1e-9);
}

TEST(thermal, uniform_power_matches_formula) {
    // Same power density everywhere -> no sideways flow, so every cell is at
    // Tamb + P / (h * A), with A the chip area in m^2.
    Design d = twoBlocks(1.0, 1.0);
    ThermalParams p;
    p.gridN = 16;
    ThermalMap m = solveThermal(d, p);
    REQUIRE(m.converged);
    double area = 2000e-6 * 1000e-6;
    double expected = p.ambientC + 2.0 / (p.heatSinkH * area);
    CHECK_NEAR(m.maxTemp, expected, 1e-3);
    CHECK_NEAR(m.avgTemp, expected, 1e-3);
}

TEST(thermal, energy_balance_and_hot_side) {
    Design d = twoBlocks(2.0, 0.0);
    ThermalParams p;
    p.gridN = 16;
    ThermalMap m = solveThermal(d, p);
    REQUIRE(m.converged);
    CHECK(m.iterations < p.maxIterations);
    // At steady state all the power leaves through the heat sink.
    double cellArea = (m.cellW * p.unitToMeters) * (m.cellH * p.unitToMeters);
    double out = 0.0;
    for (double t : m.temp) out += p.heatSinkH * cellArea * (t - p.ambientC);
    CHECK_NEAR(out, 2.0, 2.0 * 1e-3);
    // The powered (left) side is hotter, and temperature falls from left to right.
    int mid = m.n / 2;
    CHECK(m.tempAt(mid, 0) > m.tempAt(mid, m.n - 1));
    for (int c = 1; c < m.n; ++c) CHECK(m.tempAt(mid, c) <= m.tempAt(mid, c - 1) + 1e-9);
    CHECK(m.maxTemp > m.avgTemp);
}

TEST(thermal, sor_gives_the_same_answer) {
    Design d = twoBlocks(2.0, 0.5);
    ThermalParams gs;
    gs.gridN = 16;
    gs.omega = 1.0;  // plain Gauss-Seidel
    ThermalParams sor = gs;
    sor.omega = 0.0;  // automatic over-relaxation
    ThermalMap a = solveThermal(d, gs);
    ThermalMap b = solveThermal(d, sor);
    REQUIRE(a.converged && b.converged);
    CHECK(b.iterations < a.iterations);  // SOR needs fewer sweeps
    for (std::size_t i = 0; i < a.temp.size(); ++i) CHECK_NEAR(a.temp[i], b.temp[i], 1e-3);
}

// ------------------------------------------------------------------ Jose (J12)

TEST(thermal_svg, one_cell_per_grid_square) {
    Design d = twoBlocks(2.0, 0.0);
    ThermalParams p;
    p.gridN = 8;
    ThermalMap m = solveThermal(d, p);
    std::string err;
    REQUIRE(exportThermalSVG(m, tt::out("test_thermal.svg"), "heat map", err));
    std::ifstream in(tt::out("test_thermal.svg"));
    std::string s((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    CHECK(s.find("<svg") != std::string::npos);
    CHECK(s.find("heat map") != std::string::npos);
    // One <rect> per cell, tagged with class="cell" so the test can count them.
    std::size_t count = 0, pos = 0;
    while ((pos = s.find("class=\"cell\"", pos)) != std::string::npos) { ++count; ++pos; }
    CHECK_EQ(count, static_cast<std::size_t>(64));
    ThermalMap empty;
    CHECK(!exportThermalSVG(empty, tt::out("empty_thermal.svg"), "x", err));
}
