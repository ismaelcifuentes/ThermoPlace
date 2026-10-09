// ThermoPlace - command line program.
// OWNER: Eimi (integration).
//
// Usage:
//   thermoplace <blocks-file> [--out DIR] [--scale S] [--seed N] [--iters N]
// Example:
//   thermoplace benchmarks/mcnc/ami33.block --out results --iters 20000
#include <cstdio>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>

#include "thermoplace/BStarTree.hpp"
#include "thermoplace/Design.hpp"
#include "thermoplace/Exporters.hpp"
#include "thermoplace/Metrics.hpp"
#include "thermoplace/Parser.hpp"
#include "thermoplace/Validator.hpp"

using namespace tp;

namespace {

struct Options {
    std::string input;
    std::string outDir = "results";
    double scale = 1e-6;  // benchmark units -> meters (for HotSpot)
    unsigned seed = 1;
    long iters = 0;       // 0 = no optimization, only the initial packing
};

void usage() {
    std::cerr << "Usage: thermoplace <blocks-file> [--out DIR] [--scale S] [--seed N] [--iters N]\n"
              << "  e.g. thermoplace benchmarks/mcnc/ami33.block --out results --iters 20000\n";
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
        else { std::cerr << "Unknown or incomplete option: " << a << "\n"; return false; }
    }
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

    // 4) Optional quick optimization (demo for review 1)
    if (opt.iters > 0 && tree.size() > 1) {
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
        rep = validatePlacement(d);
        std::cout << "Random search: " << opt.iters << " iterations, " << improvements
                  << " improvements (seed " << opt.seed << ")\n";
        std::cout << "Validation: " << (rep.ok ? "OK (no overlaps)" : "FAILED") << "\n";
        printMetrics("best", computeMetrics(d));
        exportAll(d, opt, "best");
    }

    return rep.ok ? 0 : 3;
}
