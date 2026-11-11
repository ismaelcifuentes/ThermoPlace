// ThermoPlace - J15 helper: prepares a floorplan for HotSpot and computes our
// model's block temperatures, so both can be compared (docs/VALIDATION.md).
// OWNER: Jose. Not part of the main program; see tools/README.md.
//
// Usage:
//   j15_tool <bench.block> <best.flp> <prefix> filled
//       -> <prefix>_filled.flp and <prefix>_filled.ptrace for HotSpot
//   j15_tool <bench.block> <best.flp> <prefix> sweep
//       -> <prefix>_sweep.csv: grid 32, every (heatSinkH, packageR) pair
//   j15_tool <bench.block> <best.flp> <prefix> grids <heatSinkH> <packageR>
//       -> <prefix>_grids.csv: grids 8, 16, 32 and 64 with those two values
// The CSV files have the columns grid,heat_sink_h,package_r,block,temp_c,seconds.
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "thermoplace/Metrics.hpp"
#include "thermoplace/Parser.hpp"
#include "thermoplace/Power.hpp"
#include "thermoplace/Thermal.hpp"

using namespace tp;

namespace {

// A rectangle of the HotSpot floorplan: a real block or a zero-power filler.
struct Rect {
    std::string name;
    double x, y, w, h;  // benchmark units (micrometers)
    double power;       // watts
};

// Reads block positions from a .flp written by exportHotSpotFLP (meters).
bool readFlp(const std::string& path, Design& d) {
    std::ifstream in(path);
    if (!in) return false;
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::istringstream fields(line);
        std::string name;
        double w, h, x, y;
        if (!(fields >> name >> w >> h >> x >> y)) continue;
        for (Block& b : d.blocks) {
            if (b.name != name) continue;
            b.x = std::round(x / 1e-6);
            b.y = std::round(y / 1e-6);
            // The .flp has the placed width: if it is not the original one, it is rotated.
            b.rotated = std::fabs(w / 1e-6 - b.width) > 0.5;
        }
    }
    return true;
}

// HotSpot needs a floorplan with no empty space. We cut the die into vertical
// strips at every block edge; inside each strip, every part not covered by a
// block becomes a filler block with 0 W.
std::vector<Rect> fillDie(const Design& d, double dieW, double dieH) {
    std::vector<Rect> rects;
    for (const Block& b : d.blocks) rects.push_back({b.name, b.x, b.y, b.w(), b.h(), b.power});

    std::vector<double> xs = {0.0, dieW};
    for (const Block& b : d.blocks) {
        xs.push_back(b.x);
        xs.push_back(b.x + b.w());
    }
    std::sort(xs.begin(), xs.end());
    xs.erase(std::unique(xs.begin(), xs.end()), xs.end());

    int fillers = 0;
    for (std::size_t i = 0; i + 1 < xs.size(); ++i) {
        double xa = xs[i], xb = xs[i + 1];
        // Vertical intervals covered by blocks inside this strip, bottom to top.
        std::vector<std::pair<double, double>> covered;
        for (const Block& b : d.blocks) {
            if (b.x < xb && b.x + b.w() > xa) covered.push_back({b.y, b.y + b.h()});
        }
        std::sort(covered.begin(), covered.end());
        double y = 0.0;
        for (const auto& c : covered) {
            if (c.first > y) {
                rects.push_back({"_fill" + std::to_string(fillers++), xa, y, xb - xa, c.first - y, 0.0});
            }
            y = std::max(y, c.second);
        }
        if (y < dieH) {
            rects.push_back({"_fill" + std::to_string(fillers++), xa, y, xb - xa, dieH - y, 0.0});
        }
    }
    return rects;
}

// Solves our model once and appends one CSV row per block.
void writeRun(std::ofstream& out, const Design& d, int grid, double h, double r) {
    ThermalParams p;
    p.gridN = grid;
    p.heatSinkH = h;
    p.packageR = r;
    auto t0 = std::chrono::steady_clock::now();
    ThermalMap map = solveThermal(d, p);
    std::vector<double> temps = blockTemperatures(d, map);
    double seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
    for (std::size_t i = 0; i < d.blocks.size(); ++i) {
        out << grid << "," << h << "," << r << "," << d.blocks[i].name << "," << temps[i] << ","
            << seconds << "\n";
    }
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 5) {
        std::cerr << "usage: j15_tool <bench.block> <best.flp> <prefix> filled|sweep|grids [h r]\n";
        return 1;
    }
    const std::string prefix = argv[3];
    const std::string mode = argv[4];

    Design d;
    std::string err;
    if (!loadBenchmark(argv[1], d, err) || !readFlp(argv[2], d)) {
        std::cerr << "cannot read " << argv[1] << " or " << argv[2] << " " << err << "\n";
        return 1;
    }
    // Same power as the whole project: seed 1, 0.5 to 2 W/mm^2 (task J7).
    assignSyntheticPower(d, 1, 0.5e-6, 2e-6);

    if (mode == "filled") {
        // The die is the bounding box enlarged to the outline, like powerGrid (I13).
        Metrics m = computeMetrics(d);
        double dieW = std::max(m.chipW, d.outlineW);
        double dieH = std::max(m.chipH, d.outlineH);
        std::vector<Rect> rects = fillDie(d, dieW, dieH);

        std::ofstream flp(prefix + "_filled.flp"), ptrace(prefix + "_filled.ptrace");
        flp.precision(10);
        double area = 0.0;
        for (const Rect& r : rects) {
            flp << r.name << "\t" << r.w * 1e-6 << "\t" << r.h * 1e-6 << "\t" << r.x * 1e-6
                << "\t" << r.y * 1e-6 << "\n";
            area += r.w * r.h;
        }
        for (std::size_t i = 0; i < rects.size(); ++i) ptrace << (i ? "\t" : "") << rects[i].name;
        ptrace << "\n";
        for (std::size_t i = 0; i < rects.size(); ++i) ptrace << (i ? "\t" : "") << rects[i].power;
        ptrace << "\n";
        std::cout << d.name << ": " << d.blocks.size() << " blocks + "
                  << rects.size() - d.blocks.size() << " fillers, die covered "
                  << 100.0 * area / (dieW * dieH) << " %, power " << totalPower(d) << " W\n";
        return 0;
    }

    std::ofstream out(prefix + "_" + mode + ".csv");
    out.precision(10);
    out << "grid,heat_sink_h,package_r,block,temp_c,seconds\n";
    if (mode == "sweep") {
        // heatSinkH from 1e4 to 1e7 (20 values per decade), packageR from 0 to 1 K/W.
        for (int k = 0; k <= 60; ++k) {
            double h = 1e4 * std::pow(10.0, k / 20.0);
            for (int j = 0; j <= 50; ++j) writeRun(out, d, 32, h, j * 0.02);
        }
    } else if (mode == "grids" && argc >= 7) {
        const int grids[] = {8, 16, 32, 64};
        for (int g : grids) writeRun(out, d, g, std::atof(argv[5]), std::atof(argv[6]));
    } else {
        std::cerr << "unknown mode " << mode << "\n";
        return 1;
    }
    return 0;
}
