// ThermoPlace - command line program.
// OWNER: Eimi (integration). Follow docs/MANUAL_EIMI.md, tasks E5 and E7.
//
// Usage:
//   thermoplace <blocks-file> [--out DIR] [--scale S] [--seed N] [--iters N]
// Example:
//   thermoplace benchmarks/mcnc/ami33.block --out results --iters 20000
#include <cstdio>
#include <cstdlib>
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

    // 1) Load the benchmark ----------------------------------------------------
    Design d;
    std::string err;
    if (!loadBenchmark(opt.input, d, err)) {
        std::cerr << "Could not load benchmark: " << err << "\n";
        return 1;
    }
    std::cout << "Design " << d.name << ": " << d.blocks.size() << " blocks, "
              << d.terminals.size() << " terminals, " << d.nets.size() << " nets, outline "
              << d.outlineW << " x " << d.outlineH << ", block area " << d.totalBlockArea() << "\n";

    // 2) Initial B*-tree and packing --------------------------------------------
    BStarTree tree;
    tree.buildInitial(static_cast<int>(d.blocks.size()));
    if (!tree.isValid()) std::cout << "  [warn] B*-tree is not valid yet (Ismael's part)\n";
    tree.pack(d);

    // 3) Check legality and measure ---------------------------------------------
    ValidationReport rep = validatePlacement(d);
    std::cout << "Validation: " << (rep.ok ? "OK (no overlaps)" : "FAILED") << "\n";
    for (std::size_t i = 0; i < rep.errors.size() && i < 10; ++i)
        std::cout << "  - " << rep.errors[i] << "\n";
    Metrics m0 = computeMetrics(d);
    printMetrics("initial", m0);
    exportAll(d, opt, "initial");

    // 4) Optional quick optimization (demo for review 1) ------------------------
    if (opt.iters > 0) {
        // TODO(Eimi): task E7 - random search: perturb (swap two nodes or rotate
        // one block), pack, keep the change if the chip area improves,
        // otherwise undo it. Week 3 replaces this with simulated annealing.
        std::cout << "Random search not implemented yet (TODO Eimi, task E7)\n";
    }

    return rep.ok ? 0 : 3;
}
