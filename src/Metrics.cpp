// ThermoPlace - floorplan metrics.
// OWNER: Jose. Follow docs/MANUAL_JOSE.md. Contract: include/thermoplace/Metrics.hpp
#include "thermoplace/Metrics.hpp"

#include <algorithm>  // std::max, std::min
#include <limits>     // std::numeric_limits

namespace tp {

Metrics computeMetrics(const Design& d) {
    Metrics m;

    // 1. Bounding box: the chip ends where the farthest block ends.
    //    w()/h() already swap width and height when a block is rotated.
    for (const Block& b : d.blocks) {
        m.chipW = std::max(m.chipW, b.x + b.w());
        m.chipH = std::max(m.chipH, b.y + b.h());
    }

    // 2. Areas: the whole chip versus the sum of the blocks.
    m.chipArea = m.chipW * m.chipH;
    m.blockArea = d.totalBlockArea();

    // 3. Dead space: percentage of the chip not covered by any block.
    //    An empty chip (area 0) has no dead space; this avoids dividing by zero.
    if (m.chipArea > 0) {
        m.deadSpacePct = 100.0 * (m.chipArea - m.blockArea) / m.chipArea;
    } else {
        m.deadSpacePct = 0.0;
    }

    // 4. Wirelength (task J2).
    m.hpwl = computeHPWL(d);

    // 5. Fixed outline: an outline of 0 means "no limit" in that direction.
    bool fitsW = (d.outlineW <= 0) || (m.chipW <= d.outlineW);
    bool fitsH = (d.outlineH <= 0) || (m.chipH <= d.outlineH);
    m.fitsOutline = fitsW && fitsH;

    return m;
}

double computeHPWL(const Design& d) {
    double total = 0.0;

    for (const Net& net : d.nets) {
        // A net with fewer than 2 pins needs no wire.
        if (net.blocks.size() + net.terminals.size() < 2) continue;

        // Start the box "inside out" so the first point always replaces it.
        double minX = std::numeric_limits<double>::max();
        double minY = std::numeric_limits<double>::max();
        double maxX = -std::numeric_limits<double>::max();
        double maxY = -std::numeric_limits<double>::max();

        // Blocks connect at their center.
        for (int k : net.blocks) {
            const Block& b = d.blocks[k];
            double cx = b.x + b.w() / 2.0;
            double cy = b.y + b.h() / 2.0;
            minX = std::min(minX, cx);
            maxX = std::max(maxX, cx);
            minY = std::min(minY, cy);
            maxY = std::max(maxY, cy);
        }

        // Terminals (I/O pads) are fixed points.
        for (int t : net.terminals) {
            const Terminal& p = d.terminals[t];
            minX = std::min(minX, p.x);
            maxX = std::max(maxX, p.x);
            minY = std::min(minY, p.y);
            maxY = std::max(maxY, p.y);
        }

        // Half perimeter of the box that encloses all the pins of this net.
        total += (maxX - minX) + (maxY - minY);
    }

    return total;
}

}  // namespace tp
