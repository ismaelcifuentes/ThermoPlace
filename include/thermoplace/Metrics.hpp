// ThermoPlace - quality metrics of a packed floorplan.
// OWNER: Jose  (implementation goes in src/Metrics.cpp)
#pragma once

#include "thermoplace/Design.hpp"

namespace tp {

struct Metrics {
    double chipW = 0.0;        // bounding box width  = max(x + w) over blocks
    double chipH = 0.0;        // bounding box height = max(y + h) over blocks
    double chipArea = 0.0;     // chipW * chipH
    double blockArea = 0.0;    // sum of block areas
    double deadSpacePct = 0.0; // 100 * (chipArea - blockArea) / chipArea
    double hpwl = 0.0;         // half-perimeter wirelength (see computeHPWL)
    bool fitsOutline = true;   // chipW <= outlineW && chipH <= outlineH (true if no outline)
};

// Computes all fields above for the current block positions.
Metrics computeMetrics(const Design& d);

// Half-Perimeter WireLength: for every net, take the bounding box of the
// CENTERS of its blocks (x + w/2, y + h/2) and the positions of its terminals,
// and add (maxX - minX) + (maxY - minY). Nets with < 2 pins add 0.
double computeHPWL(const Design& d);

}  // namespace tp
