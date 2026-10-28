// ThermoPlace - command line program.
// OWNER: Eimi (integration).
//
// Usage:
//   thermoplace <blocks-file> [--out DIR] [--scale S] [--seed N] [--iters N]
//               [--sa] [--alpha A] [--outline-weight W] [--cooling C] [--moves-per-temp N]
//               [--runs-csv FILE]
// Examples:
//   thermoplace benchmarks/mcnc/ami33.block --out results --iters 20000
//   thermoplace benchmarks/mcnc/ami33.block --sa --alpha 0.5 --seed 1
//   thermoplace benchmarks/mcnc/ami49.block --sa --outline-weight 5 --cooling 0.98 --moves-per-temp 2000
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>

#include "thermoplace/Annealer.hpp"
#include "thermoplace/BStarTree.hpp"
#include "thermoplace/Cost.hpp"
#include "thermoplace/Design.hpp"
#include "thermoplace/Exporters.hpp"
#include "thermoplace/Metrics.hpp"
#include "thermoplace/Parser.hpp"
#include "thermoplace/Report.hpp"
#include "thermoplace/Validator.hpp"

using namespace tp;

namespace {

struct Options {
    std::string input;
    std::string outDir = "results";
    double scale = 1e-6;  // benchmark units -> meters (for HotSpot)
    unsigned seed = 1;
    long iters = 0;       // 0 = no optimization, only the initial packing
    bool sa = false;      // E8: simulated annealing instead of random search
    double alpha = 0.5;
    double outlineWeight = 0.0;
    double cooling = 0.95;
    int movesPerTemp = 0; // 0 = automatic (10 x number of blocks), as in SAParams
    std::string runsCsv;  // empty = <out>/runs.csv
};

void usage() {
    std::cerr << "Usage: thermoplace <blocks-file> [--out DIR] [--scale S] [--seed N] [--iters N]\n"
              << "                   [--sa] [--alpha A] [--outline-weight W] [--cooling C] [--moves-per-temp N]\n"
              << "                   [--runs-csv FILE]\n"
              << "  e.g. thermoplace benchmarks/mcnc/ami33.block --out results --iters 20000\n"
              << "       thermoplace benchmarks/mcnc/ami33.block --sa --alpha 0.5 --seed 1\n"
              << "  With --sa, --iters is ignored. With --outline-weight above 0, SA returns the best\n"
              << "  floorplan that fits the outline, if it found one.\n";
}

bool parseArgs(int argc, char** argv, Options& o) {
    if (argc < 2) return false;
    o.input = argv[1];
    for (int i = 2; i < argc; ++i) {
        std::string a = argv[i];
        auto next = [&](void) -> const char* { return (i + 1 < argc) ? argv[++i] : nullptr; };
        const char* v = nullptr;
        if (a == "--out" && (v = next())) o.outDir = v;
        else if (a == "--scale" && (v = next())) o.scale = std::atof(v);
        else if (a == "--seed" && (v = next())) o.seed = static_cast<unsigned>(std::atol(v));
        else if (a == "--iters" && (v = next())) o.iters = std::atol(v);
        else if (a == "--sa") o.sa = true;
        else if (a == "--alpha" && (v = next())) o.alpha = std::atof(v);
        else if (a == "--outline-weight" && (v = next())) o.outlineWeight = std::atof(v);
        else if (a == "--cooling" && (v = next())) o.cooling = std::atof(v);
        else if (a == "--moves-per-temp" && (v = next())) o.movesPerTemp = std::atoi(v);
        else if (a == "--runs-csv" && (v = next())) o.runsCsv = v;
        else { std::cerr << "Unknown or incomplete option: " << a << "\n"; return false; }
    }
    if (o.runsCsv.empty()) o.runsCsv = o.outDir + "/runs.csv";
    // atof returns 0 on garbage, so a typo like "--alpha O.5" must not slip through as 0.
    if (o.alpha < 0.0 || o.alpha > 1.0) { std::cerr << "--alpha must be between 0 and 1\n"; return false; }
    if (o.cooling <= 0.0 || o.cooling >= 1.0) { std::cerr << "--cooling must be between 0 and 1 (exclusive)\n"; return false; }
    if (o.outlineWeight < 0.0) { std::cerr << "--outline-weight cannot be negative\n"; return false; }
    if (o.movesPerTemp < 0) { std::cerr << "--moves-per-temp cannot be negative (0 = automatic)\n"; return false; }
    return true;
}

void printMetrics(const char* label, const Metrics& m) {
    std::printf("%-10s chip %.0f x %.0f | area %.0f | dead space %.2f%% | HPWL %.0f | fits outline: %s\n",
                label, m.chipW, m.chipH, m.chipArea, m.deadSpacePct, m.hpwl,
                m.fitsOutline ? "yes" : "no");
}

void exportAll(const Design& d, const Options& o, const std::string& tag) {
    std::string base = o.outDir + "/" + d.name + "_" + tag;
    std::string err;
    if (exportSVG(d, base + ".svg", err)) std::cout << "  wrote " << base << ".svg\n";
    else std::cout << "  [skip] SVG: " << err << "\n";
    if (exportHotSpotFLP(d, base + ".flp", o.scale, err)) std::cout << "  wrote " << base << ".flp\n";
    else std::cout << "  [skip] FLP: " << err << "\n";
}

// One line of the runs table, so random search and SA land in the same CSV and
// the paper can compare them seed by seed.
void recordRun(const Design& d, const Options& o, const std::string& method, double seconds) {
    Metrics m = computeMetrics(d);
    RunRecord r;
    r.benchmark = d.name;
    r.method = method;
    r.seed = o.seed;
    r.alpha = o.alpha;
    r.outlineWeight = o.outlineWeight;
    r.chipArea = m.chipArea;
    r.deadSpacePct = m.deadSpacePct;
    r.hpwl = m.hpwl;
    r.fitsOutline = m.fitsOutline;
    r.seconds = seconds;
    std::string err;
    if (appendRunCSV(r, o.runsCsv, err)) std::cout << "  added a line to " << o.runsCsv << "\n";
    else std::cout << "  [skip] runs CSV: " << err << "\n";
}

// RunRecord has no columns for cooling or moves per temperature, so two SA setups
// would land in runs.csv under the same "sa" and could not be told apart in the
// table. Non-default settings go into the method name instead, e.g. "sa_c0.98_m2000".
std::string saMethodName(const Options& o) {
    if (o.cooling == 0.95 && o.movesPerTemp == 0) return "sa";
    char buf[64];
    std::snprintf(buf, sizeof buf, "sa_c%g_m%d", o.cooling, o.movesPerTemp);
    return buf;
}

double secondsSince(std::chrono::steady_clock::time_point t0) {
    return std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
}

}  // namespace

int main(int argc, char** argv) {
    Options opt;
    if (!parseArgs(argc, argv, opt)) { usage(); return 2; }

    // 1) Load the benchmark
    Design d;
    std::string err;
    if (!loadBenchmark(opt.input, d, err)) {
        std::cerr << "Could not load benchmark: " << err << "\n";
        return 1;
    }
    // Fixed notation: the default would print ami33's block area as 1.15645e+06.
    std::cout << std::fixed << std::setprecision(0);
    std::cout << "Design " << d.name << ": " << d.blocks.size() << " blocks, "
              << d.terminals.size() << " terminals, " << d.nets.size() << " nets, outline "
              << d.outlineW << " x " << d.outlineH << ", block area " << d.totalBlockArea() << "\n";

    // 2) Initial B*-tree and packing
    BStarTree tree;
    tree.buildInitial(static_cast<int>(d.blocks.size()));
    if (!tree.isValid()) std::cout << "  [warn] B*-tree is not valid yet (Ismael's part)\n";
    tree.pack(d);

    // 3) Check legality and measure
    ValidationReport rep = validatePlacement(d);
    std::cout << "Validation: " << (rep.ok ? "OK (no overlaps)" : "FAILED") << "\n";
    for (std::size_t i = 0; i < rep.errors.size() && i < 10; ++i)
        std::cout << "  - " << rep.errors[i] << "\n";
    Metrics m0 = computeMetrics(d);
    printMetrics("initial", m0);
    exportAll(d, opt, "initial");

    // 4a) E8. Simulated annealing (Ismael's anneal + Jose's computeCost).
    if (opt.sa && tree.size() > 1) {
        // Area and HPWL are normalized by the initial floorplan, so alpha weighs two
        // numbers that both start near 1 instead of 4.6 million against 220 thousand.
        CostNorm norm = makeNorm(m0);
        CostWeights w;
        w.alpha = opt.alpha;
        w.outline = opt.outlineWeight;
        CostFunction cost = [&](const Design& x) { return computeCost(computeMetrics(x), x, w, norm); };

        SAParams p;
        p.seed = opt.seed;
        p.cooling = opt.cooling;
        p.movesPerTemp = opt.movesPerTemp;
        // E8b. Only with an outline penalty does "fits the outline" mean anything to the
        // cost; then the answer should be the best floorplan that fits, not a slightly
        // cheaper one that sticks out (Ismael's I9).
        bool wantFeasible = opt.outlineWeight > 0.0;
        if (wantFeasible) p.feasible = [](const Design& x) { return computeMetrics(x).fitsOutline; };

        auto t0 = std::chrono::steady_clock::now();
        SAResult res = anneal(d, tree, cost, p);
        double secs = secondsSince(t0);

        rep = validatePlacement(d);
        std::printf("Simulated annealing: %d temperatures, %ld moves, %ld accepted (%.1f%%), "
                    "cost %.4f to %.4f, %.2f s (seed %u, alpha %.2f)\n",
                    res.temperatures, res.moves, res.accepted,
                    res.moves > 0 ? 100.0 * res.accepted / res.moves : 0.0,
                    res.initialCost, res.bestCost, secs, opt.seed, opt.alpha);
        if (wantFeasible)
            std::cout << "Fits outline: " << (res.feasibleFound ? "found a floorplan that fits"
                                                                : "no floorplan that fits was found")
                      << "\n";
        std::cout << "Validation: " << (rep.ok ? "OK (no overlaps)" : "FAILED") << "\n";
        printMetrics("best", computeMetrics(d));
        exportAll(d, opt, "best");

        std::string base = opt.outDir + "/" + d.name;
        if (writeTraceCSV(res.trace, base + "_trace.csv", err)) std::cout << "  wrote " << base << "_trace.csv\n";
        else std::cout << "  [skip] trace CSV: " << err << "\n";
        std::string title = d.name + " SA (seed " + std::to_string(opt.seed) + ")";
        if (exportConvergenceSVG(res.trace, base + "_convergence.svg", title, err))
            std::cout << "  wrote " << base << "_convergence.svg\n";
        else std::cout << "  [skip] convergence SVG: " << err << "\n";
        recordRun(d, opt, saMethodName(opt), secs);
        return rep.ok ? 0 : 3;
    }

    // 4b) Random search (review 1 baseline), kept so SA has something to beat.
    if (opt.iters > 0 && tree.size() > 1) {
        auto t0 = std::chrono::steady_clock::now();
        // E7. Stand-in for simulated annealing (week 3): a change is kept only if the
        // chip area shrinks. It stalls in local minima, which is exactly what SA fixes,
        // so the before/after numbers here are the baseline SA has to beat.
        std::mt19937 rng(opt.seed);
        std::uniform_int_distribution<int> anyNode(0, tree.size() - 1);
        std::bernoulli_distribution doSwap(0.5);

        double bestArea = m0.chipArea;
        long improvements = 0;
        for (long it = 0; it < opt.iters; ++it) {
            BStarTree candidate = tree;
            int rotated = -1;
            if (doSwap(rng)) {
                candidate.swapBlocks(anyNode(rng), anyNode(rng));
            } else {
                rotated = candidate.node(anyNode(rng)).block;
                d.blocks[rotated].rotated = !d.blocks[rotated].rotated;
            }

            candidate.pack(d);
            double area = computeMetrics(d).chipArea;
            if (area < bestArea) {
                bestArea = area;
                tree = candidate;
                ++improvements;
            } else if (rotated >= 0) {
                // Rotation is stored in the Design, not in the tree, so a rejected
                // rotation has to be undone by hand.
                d.blocks[rotated].rotated = !d.blocks[rotated].rotated;
            }
        }

        // d still holds the positions of the last candidate tried, not of the best tree.
        tree.pack(d);
        double secs = secondsSince(t0);
        rep = validatePlacement(d);
        std::cout << "Random search: " << opt.iters << " iterations, " << improvements
                  << " improvements (seed " << opt.seed << ")\n";
        std::cout << "Validation: " << (rep.ok ? "OK (no overlaps)" : "FAILED") << "\n";
        printMetrics("best", computeMetrics(d));
        exportAll(d, opt, "best");
        recordRun(d, opt, "random", secs);
    }

    return rep.ok ? 0 : 3;
}
