// ThermoPlace - output files (pictures and thermal-simulator input).
// OWNER: Jose  (implementation goes in src/Exporters.cpp)
#pragma once

#include <string>

#include <vector>

#include "thermoplace/Annealer.hpp"
#include "thermoplace/Design.hpp"

namespace tp {

// Writes an SVG picture of the floorplan: one rectangle per block with its
// name, the bounding box, and a title with design name, area and dead space.
// Remember that SVG's y axis points DOWN: draw y' = chipH - (y + h).
bool exportSVG(const Design& d, const std::string& path, std::string& err);

// Writes a HotSpot floorplan file (.flp). One line per block:
//     <name>\t<width>\t<height>\t<left-x>\t<bottom-y>
// in METERS. `unitToMeters` converts benchmark units (1e-6 = micrometers).
// Format verified against HotSpot v7 examples/example1/ev6.flp.
bool exportHotSpotFLP(const Design& d, const std::string& path,
                      double unitToMeters, std::string& err);

// EXTRA (week 5): HotSpot power trace (.ptrace): first line = block names
// separated by tabs, second line = Block::power of each block (watts).
bool exportPTrace(const Design& d, const std::string& path, std::string& err);

// EXTRA (week 5): writes a CSV "name,x,y,w,h,rotated,power" for plotting and
// for the benchmark tables of the paper.
bool exportCSV(const Design& d, const std::string& path, std::string& err);

// WEEK 2: convergence plot of simulated annealing as an SVG line chart.
// x = temperature step, y = cost. Draw TWO <polyline> elements: current cost
// and best cost (in different colors), axes with the min and max cost written,
// and a title. Returns false if the trace is empty or the file cannot be opened.
bool exportConvergenceSVG(const std::vector<SATracePoint>& trace, const std::string& path,
                          const std::string& title, std::string& err);

}  // namespace tp
