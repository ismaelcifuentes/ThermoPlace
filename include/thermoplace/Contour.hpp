// ThermoPlace - horizontal contour used while packing a B*-tree.
// OWNER: Ismael  (implementation goes in src/Contour.cpp)
//
// The contour is the "skyline" of the blocks placed so far. When a new block
// is placed at [x1, x2) its y coordinate is the highest point of the skyline
// inside that interval; then the skyline is raised to y + height there.
//
// Version for review 1: keep a list of segments and scan it (O(n) per query,
// O(n^2) per packing). That is fast enough for n <= 300.
// Week 7 optimization (good for the paper): doubly linked list contour,
// amortized O(1) per block -> O(n) packing, as in Chang et al. (DAC 2000).
#pragma once

#include <vector>

namespace tp {

class Contour {
public:
    // Removes every segment (empty skyline = height 0 everywhere).
    void clear();

    // Highest top among the segments that overlap the open interval (x1, x2).
    // Segments that only touch at an end point do NOT overlap.
    // Returns 0 if nothing overlaps.
    double maxHeight(double x1, double x2) const;

    // Registers a block that occupies [x1, x2) and whose top edge is at `top`.
    void insert(double x1, double x2, double top);

    // Number of stored segments (useful for tests/debugging).
    int size() const { return static_cast<int>(segs_.size()); }

private:
    struct Segment {
        double x1, x2, top;
    };
    std::vector<Segment> segs_;
};

}  // namespace tp
