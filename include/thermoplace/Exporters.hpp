// ThermoPlace - output files (pictures and thermal-simulator input).
// OWNER: Jose  (implementation goes in src/Exporters.cpp)
#pragma once

#include <string>

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

}  // namespace tp
