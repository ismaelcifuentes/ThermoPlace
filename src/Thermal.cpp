// ThermoPlace - fast steady-state thermal model.
// OWNER: Ismael. Follow docs/SEMANA4.md. Contract: include/thermoplace/Thermal.hpp
#include "thermoplace/Thermal.hpp"

#include <algorithm>
#include <cmath>

namespace tp {

namespace {

// Length of the overlap between [a1, a2) and [b1, b2) (0 if they do not touch).
double overlap(double a1, double a2, double b1, double b2) {
    return std::max(0.0, std::min(a2, b2) - std::max(a1, b1));
}

}  // namespace

// I11: each block shares its power among the cells it covers, in proportion
// to the covered area. Only the cells under the block are visited.
std::vector<double> powerGrid(const Design& d, int n, double& cellW, double& cellH) {
    std::vector<double> grid(n > 0 ? n * n : 0, 0.0);
    double chipW = 0.0, chipH = 0.0;
    for (const Block& b : d.blocks) {
        chipW = std::max(chipW, b.x + b.w());
        chipH = std::max(chipH, b.y + b.h());
    }
    cellW = (n > 0) ? chipW / n : 0.0;
    cellH = (n > 0) ? chipH / n : 0.0;
    if (n <= 0 || cellW <= 0.0 || cellH <= 0.0) return grid;

    for (const Block& b : d.blocks) {
        double area = b.w() * b.h();
        if (b.power == 0.0 || area <= 0.0) continue;
        double density = b.power / area;  // watts per unit^2
        // Range of columns and rows touched by the block (clamped to the grid).
        int c0 = std::max(0, static_cast<int>(b.x / cellW));
        int c1 = std::min(n - 1, static_cast<int>((b.x + b.w()) / cellW));
        int r0 = std::max(0, static_cast<int>(b.y / cellH));
        int r1 = std::min(n - 1, static_cast<int>((b.y + b.h()) / cellH));
        for (int r = r0; r <= r1; ++r) {
            double oy = overlap(b.y, b.y + b.h(), r * cellH, (r + 1) * cellH);
            for (int c = c0; c <= c1; ++c) {
                double ox = overlap(b.x, b.x + b.w(), c * cellW, (c + 1) * cellW);
                grid[r * n + c] += density * ox * oy;
            }
        }
    }
    return grid;
}

// I11: Gauss-Seidel with over-relaxation (SOR) on the heat-balance equation of every cell (see the header).
ThermalMap solveThermal(const Design& d, const ThermalParams& p) {
    ThermalMap map;
    const int n = p.gridN;
    map.n = n;
    map.power = powerGrid(d, n, map.cellW, map.cellH);
    map.temp.assign(map.power.size(), p.ambientC);
    if (map.power.empty() || map.cellW <= 0.0 || map.cellH <= 0.0) {
        map.maxTemp = map.avgTemp = p.ambientC;
        map.converged = true;
        return map;
    }

    // Conductances in W/K. A sideways path between two cells has cross-section
    // (shared edge * thickness) and length (distance between cell centres).
    const double w = map.cellW * p.unitToMeters;
    const double h = map.cellH * p.unitToMeters;
    const double gx = p.siliconK * p.dieThickness * h / w;  // left-right neighbours
    const double gy = p.siliconK * p.dieThickness * w / h;  // down-up neighbours
    const double gv = p.heatSinkH * w * h;                 // cell -> ambient
    const double pi = 3.14159265358979323846;
    const double omega = (p.omega > 0.0) ? p.omega : 2.0 / (1.0 + std::sin(pi / (n + 1)));

    for (int it = 0; it < p.maxIterations; ++it) {
        double maxChange = 0.0;
        for (int r = 0; r < n; ++r) {
            for (int c = 0; c < n; ++c) {
                // Heat balance solved for T: weighted average of the neighbours
                // and the ambient, plus the cell's own power.
                double num = map.power[r * n + c] + gv * p.ambientC;
                double den = gv;
                if (c > 0)     { num += gx * map.temp[r * n + c - 1];   den += gx; }
                if (c < n - 1) { num += gx * map.temp[r * n + c + 1];   den += gx; }
                if (r > 0)     { num += gy * map.temp[(r - 1) * n + c]; den += gy; }
                if (r < n - 1) { num += gy * map.temp[(r + 1) * n + c]; den += gy; }
                double old = map.temp[r * n + c];
                double t = old + omega * (num / den - old);  // SOR step (omega = 1: plain GS)
                maxChange = std::max(maxChange, std::fabs(t - old));
                map.temp[r * n + c] = t;  // used right away by the next cells (Gauss-Seidel)
            }
        }
        map.iterations = it + 1;
        if (maxChange < p.tolerance) {
            map.converged = true;
            break;
        }
    }

    double sum = 0.0;
    map.maxTemp = map.temp[0];
    for (double t : map.temp) {
        sum += t;
        map.maxTemp = std::max(map.maxTemp, t);
    }
    map.avgTemp = sum / map.temp.size();
    return map;
}

}  // namespace tp
