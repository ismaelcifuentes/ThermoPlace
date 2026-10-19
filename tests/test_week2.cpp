// Week 2 tests: simulated annealing (Ismael), cost + convergence plot (Jose),
// CSV reports (Eimi). Run: thermoplace_tests week2   (or anneal / cost / convergence / report)
#include <fstream>
#include <random>
#include <string>
#include <vector>

#include "test_framework.hpp"
#include "thermoplace/Annealer.hpp"
#include "thermoplace/BStarTree.hpp"
#include "thermoplace/Cost.hpp"
#include "thermoplace/Exporters.hpp"
#include "thermoplace/Metrics.hpp"
#include "thermoplace/Report.hpp"

using namespace tp;

namespace {

Design randomDesign(int n, unsigned seed) {
    std::mt19937 rng(seed);
    std::uniform_real_distribution<double> size(1.0, 20.0);
    Design d;
    d.name = "rand" + std::to_string(n);
    for (int i = 0; i < n; ++i) d.addBlock("r" + std::to_string(i), size(rng), size(rng));
    return d;
}

// Bounding-box area of the current packing (independent of Jose's Metrics).
double bboxArea(const Design& d) {
    double w = 0.0, h = 0.0;
    for (const Block& b : d.blocks) {
        if (b.x + b.w() > w) w = b.x + b.w();
        if (b.y + b.h() > h) h = b.y + b.h();
    }
    return w * h;
}

bool noOverlaps(const Design& d) {
    const double eps = 1e-9;
    for (std::size_t i = 0; i < d.blocks.size(); ++i) {
        const Block& a = d.blocks[i];
        for (std::size_t j = i + 1; j < d.blocks.size(); ++j) {
            const Block& b = d.blocks[j];
            bool sepX = a.x + a.w() <= b.x + eps || b.x + b.w() <= a.x + eps;
            bool sepY = a.y + a.h() <= b.y + eps || b.y + b.h() <= a.y + eps;
            if (!sepX && !sepY) return false;
        }
    }
    return true;
}

std::vector<std::string> readLines(const std::string& path) {
    std::vector<std::string> lines;
    std::ifstream in(path);
    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        lines.push_back(line);
    }
    return lines;
}

std::string readAll(const std::string& path) {
    std::ifstream in(path);
    std::string s, line;
    while (std::getline(in, line)) s += line + "\n";
    return s;
}

SAParams quickParams(unsigned seed) {
    SAParams p;
    p.seed = seed;
    p.cooling = 0.9;
    p.movesPerTemp = 100;
    p.minTempRatio = 1e-3;
    return p;
}

}  // namespace

// ---------------------------------------------------------------- Ismael (I8)

TEST(anneal, improves_area_and_stays_legal) {
    Design d = randomDesign(30, 7);
    BStarTree t;
    t.buildInitial(30);
    t.pack(d);
    double before = bboxArea(d);
    SAResult r = anneal(d, t, bboxArea, quickParams(1));
    CHECK(r.moves > 0);
    CHECK(r.temperatures > 0);
    CHECK(r.bestCost <= r.initialCost);
    CHECK(r.bestCost < 0.9 * before);  // SA must clearly beat the initial tree
    CHECK(t.isValid());
    CHECK(noOverlaps(d));
}

TEST(anneal, design_holds_the_best_solution) {
    Design d = randomDesign(20, 3);
    BStarTree t;
    t.buildInitial(20);
    t.pack(d);
    SAResult r = anneal(d, t, bboxArea, quickParams(5));
    CHECK_NEAR(bboxArea(d), r.bestCost, 1e-6);  // d is packed with the best tree
    t.pack(d);                                  // re-packing the returned tree gives the same
    CHECK_NEAR(bboxArea(d), r.bestCost, 1e-6);
}

TEST(anneal, same_seed_same_result) {
    Design d1 = randomDesign(20, 11), d2 = randomDesign(20, 11);
    BStarTree t1, t2;
    t1.buildInitial(20);
    t2.buildInitial(20);
    t1.pack(d1);
    t2.pack(d2);
    SAResult r1 = anneal(d1, t1, bboxArea, quickParams(42));
    SAResult r2 = anneal(d2, t2, bboxArea, quickParams(42));
    CHECK(r1.moves > 0);
    CHECK_NEAR(r1.bestCost, r2.bestCost, 1e-9);
    CHECK_EQ(r1.moves, r2.moves);
}

TEST(anneal, trace_is_consistent) {
    Design d = randomDesign(15, 2);
    BStarTree t;
    t.buildInitial(15);
    t.pack(d);
    SAResult r = anneal(d, t, bboxArea, quickParams(3));
    REQUIRE(!r.trace.empty());
    CHECK_EQ(static_cast<int>(r.trace.size()), r.temperatures);
    for (std::size_t i = 0; i < r.trace.size(); ++i) {
        const SATracePoint& p = r.trace[i];
        CHECK_EQ(p.step, static_cast<int>(i));
        CHECK(p.acceptRate >= 0.0 && p.acceptRate <= 1.0);
        CHECK(p.bestCost <= p.currentCost + 1e-9);
        if (i > 0) {
            CHECK(p.temperature < r.trace[i - 1].temperature);  // it cools down
            CHECK(p.bestCost <= r.trace[i - 1].bestCost + 1e-9);  // best never gets worse
        }
    }
    CHECK_NEAR(r.trace.back().bestCost, r.bestCost, 1e-9);
}

// ------------------------------------------------------------------ Jose (J8)

namespace {
Design costDesign() {
    Design d;
    d.addBlock("a", 4, 2);
    d.outlineW = 10;
    d.outlineH = 5;
    return d;
}
Metrics fakeMetrics(double w, double h, double hpwl) {
    Metrics m;
    m.chipW = w;
    m.chipH = h;
    m.chipArea = w * h;
    m.hpwl = hpwl;
    return m;
}
}  // namespace

TEST(cost, norm_from_metrics) {
    CostNorm n = makeNorm(fakeMetrics(10, 20, 300));
    CHECK_NEAR(n.area, 200, 1e-9);
    CHECK_NEAR(n.hpwl, 300, 1e-9);
    CostNorm z = makeNorm(fakeMetrics(0, 0, 0));  // never divide by zero
    CHECK_NEAR(z.area, 1, 1e-9);
    CHECK_NEAR(z.hpwl, 1, 1e-9);
}

TEST(cost, outline_penalty) {
    Design d = costDesign();
    CHECK_NEAR(outlinePenalty(fakeMetrics(8, 5, 0), d), 0, 1e-12);       // fits
    CHECK_NEAR(outlinePenalty(fakeMetrics(15, 5, 0), d), 0.25, 1e-12);   // (5/10)^2
    CHECK_NEAR(outlinePenalty(fakeMetrics(15, 10, 0), d), 1.25, 1e-12);  // 0.25 + (5/5)^2
    Design free = costDesign();
    free.outlineW = 0;
    free.outlineH = 0;
    CHECK_NEAR(outlinePenalty(fakeMetrics(100, 100, 0), free), 0, 1e-12);  // no outline
}

TEST(cost, weighted_sum) {
    Design d = costDesign();
    CostNorm n;
    n.area = 100;
    n.hpwl = 50;
    Metrics m = fakeMetrics(20, 10, 100);  // area 200 -> 2.0, hpwl 100 -> 2.0, penalty 2.0
    CostWeights onlyArea;
    onlyArea.alpha = 1.0;
    CHECK_NEAR(computeCost(m, d, onlyArea, n), 2.0, 1e-12);
    CostWeights onlyWire;
    onlyWire.alpha = 0.0;
    CHECK_NEAR(computeCost(fakeMetrics(20, 10, 25), d, onlyWire, n), 0.5, 1e-12);
    CostWeights mix;
    mix.alpha = 0.25;
    mix.outline = 3.0;
    // 0.25*2 + 0.75*2 + 3*2 = 8
    CHECK_NEAR(computeCost(m, d, mix, n), 8.0, 1e-12);
}

// ------------------------------------------------------------------ Jose (J9)

TEST(convergence, writes_two_polylines) {
    std::vector<SATracePoint> tr;
    for (int i = 0; i < 5; ++i) {
        SATracePoint p;
        p.step = i;
        p.temperature = 100.0 / (i + 1);
        p.currentCost = 10.0 - i;
        p.bestCost = 9.0 - i;
        p.acceptRate = 0.5;
        tr.push_back(p);
    }
    std::string err;
    REQUIRE(exportConvergenceSVG(tr, tt::out("test_convergence.svg"), "test run", err));
    std::string s = readAll(tt::out("test_convergence.svg"));
    CHECK(s.find("<svg") != std::string::npos);
    std::size_t first = s.find("<polyline");
    CHECK(first != std::string::npos);
    CHECK(first != std::string::npos && s.find("<polyline", first + 1) != std::string::npos);
    CHECK(s.find("test run") != std::string::npos);
    CHECK(!exportConvergenceSVG({}, tt::out("empty.svg"), "x", err));  // empty trace -> false
}

// ------------------------------------------------------------------ Eimi (E9)

TEST(report, trace_csv) {
    std::vector<SATracePoint> tr(3);
    for (int i = 0; i < 3; ++i) {
        tr[i].step = i;
        tr[i].temperature = 10.0 - i;
        tr[i].currentCost = 2.0;
        tr[i].bestCost = 1.5;
        tr[i].acceptRate = 0.25;
    }
    std::string err;
    REQUIRE(writeTraceCSV(tr, tt::out("test_trace.csv"), err));
    std::vector<std::string> lines = readLines(tt::out("test_trace.csv"));
    REQUIRE(lines.size() == 4);
    CHECK_EQ(lines[0], std::string("step,temperature,current_cost,best_cost,accept_rate"));
    CHECK(lines[1].rfind("0,", 0) == 0);
    CHECK(lines[3].rfind("2,", 0) == 0);
    REQUIRE(writeTraceCSV(tr, tt::out("test_trace.csv"), err));  // overwrites, not appends
    CHECK_EQ(readLines(tt::out("test_trace.csv")).size(), static_cast<std::size_t>(4));
    CHECK(!writeTraceCSV(tr, "no_such_dir_xyz/a.csv", err));
}

TEST(report, runs_csv_appends_with_one_header) {
    const std::string path = tt::out("test_runs.csv");
    { std::ofstream wipe(path); }  // start from an empty file
    RunRecord r;
    r.benchmark = "ami33";
    r.method = "sa";
    r.seed = 3;
    r.fitsOutline = true;
    std::string err;
    REQUIRE(appendRunCSV(r, path, err));
    r.seed = 4;
    r.fitsOutline = false;
    REQUIRE(appendRunCSV(r, path, err));
    std::vector<std::string> lines = readLines(path);
    REQUIRE(lines.size() == 3);
    CHECK_EQ(lines[0], std::string("benchmark,method,seed,alpha,outline_weight,chip_area,"
                                   "dead_space_pct,hpwl,fits_outline,seconds"));
    CHECK(lines[1].rfind("ami33,sa,3,", 0) == 0);
    CHECK(lines[2].rfind("ami33,sa,4,", 0) == 0);
    CHECK(lines[1].find(",1,") != std::string::npos);  // fits_outline = 1
    CHECK(lines[2].find(",0,") != std::string::npos);
}
