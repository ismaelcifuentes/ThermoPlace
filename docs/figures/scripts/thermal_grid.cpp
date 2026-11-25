// ThermoPlace - figure helper: dumps the temperature of every grid cell and the
// blocks of one floorplan, so make_figures.py can draw the heat map (figure 3).
// OWNER: Jose. Not part of the program; see docs/figures/README.md.
//
// Usage: thermal_grid <bench.block> <best.flp> <prefix> <powerMin> <powerMax>
//   powerMin, powerMax: power density range in W/mm^2 (seed 1, task J7)
// Writes <prefix>_cells.csv (row,col,x,y,w,h,temp_c) and
//        <prefix>_blocks.csv (name,x,y,w,h,power_w,density_w_mm2), in micrometers.
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "thermoplace/Parser.hpp"
#include "thermoplace/Power.hpp"
#include "thermoplace/Thermal.hpp"

using namespace tp;

int main(int argc, char** argv) {
    if (argc < 6) {
        std::cerr << "usage: thermal_grid <bench.block> <best.flp> <prefix> <powerMin> <powerMax>\n";
        return 1;
    }
    Design d;
    std::string err;
    if (!loadBenchmark(argv[1], d, err)) {
        std::cerr << err << "\n";
        return 1;
    }

    // Block positions from the .flp written by the program (meters -> micrometers).
    std::ifstream flp(argv[2]);
    std::string line;
    while (std::getline(flp, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::istringstream fields(line);
        std::string name;
        double w, h, x, y;
        if (!(fields >> name >> w >> h >> x >> y)) continue;
        for (Block& b : d.blocks) {
            if (b.name != name) continue;
            b.x = std::round(x / 1e-6);
            b.y = std::round(y / 1e-6);
            b.rotated = std::fabs(w / 1e-6 - b.width) > 0.5;
        }
    }

    // Same power as the program: seed 1, uniform density (1 W/mm^2 = 1e-6 W/um^2).
    assignSyntheticPower(d, 1, std::atof(argv[4]) * 1e-6, std::atof(argv[5]) * 1e-6);

    // Same thermal model and defaults as the program (grid 32, v1.0 calibration).
    ThermalMap map = solveThermal(d, ThermalParams());

    const std::string prefix = argv[3];
    std::ofstream cells(prefix + "_cells.csv");
    cells << "row,col,x,y,w,h,temp_c\n";
    for (int r = 0; r < map.n; ++r) {
        for (int c = 0; c < map.n; ++c) {
            cells << r << "," << c << "," << c * map.cellW << "," << r * map.cellH << ","
                  << map.cellW << "," << map.cellH << "," << map.tempAt(r, c) << "\n";
        }
    }
    std::ofstream blocks(prefix + "_blocks.csv");
    blocks << "name,x,y,w,h,power_w,density_w_mm2\n";
    for (const Block& b : d.blocks) {
        blocks << b.name << "," << b.x << "," << b.y << "," << b.w() << "," << b.h() << ","
               << b.power << "," << b.power / (b.area() * 1e-6) << "\n";
    }
    std::cout << d.name << ": Tmax " << map.maxTemp << " C, total power " << totalPower(d)
              << " W\n";
    return 0;
}
