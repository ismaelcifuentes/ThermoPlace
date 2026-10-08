// ThermoPlace - contour (skyline) for B*-tree packing.
// OWNER: Ismael. Follow docs/MANUAL_ISMAEL.md. Contract: include/thermoplace/Contour.hpp
#include "thermoplace/Contour.hpp"

namespace tp {

// I1: an empty skyline has height 0 everywhere, so we just drop all segments.
void Contour::clear() {
    segs_.clear();
}

// I1: highest top among the segments that overlap the open interval (x1, x2).
// Two intervals overlap when each one starts before the other one ends.
// If they only touch at one point (s.x2 == x1 or s.x1 == x2) they do NOT overlap.
double Contour::maxHeight(double x1, double x2) const {
    double best = 0.0;  // nothing below -> the block rests on the floor (y = 0)
    for (const Segment& s : segs_) {
        bool overlaps = (s.x1 < x2) && (x1 < s.x2);
        if (overlaps && s.top > best) {
            best = s.top;
        }
    }
    return best;
}

// I1: remember that a block now covers [x1, x2) up to height `top`.
// Simple version: one segment per placed block (O(n) per query).
void Contour::insert(double x1, double x2, double top) {
    segs_.push_back({x1, x2, top});
}

}  // namespace tp
