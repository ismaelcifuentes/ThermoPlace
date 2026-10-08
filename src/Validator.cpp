// ThermoPlace - placement legality checks.
// OWNER: Jose. Follow docs/MANUAL_JOSE.md. Contract: include/thermoplace/Validator.hpp
#include "thermoplace/Validator.hpp"

namespace tp {

ValidationReport validatePlacement(const Design& d, double eps) {
    ValidationReport r;  // starts with ok = true and no errors
    const int n = static_cast<int>(d.blocks.size());

    // 1. Check each block on its own: real size and position inside the chip.
    for (const Block& b : d.blocks) {
        if (b.width <= 0 || b.height <= 0) {
            r.ok = false;
            r.errors.push_back("empty block: " + b.name);
        }
        if (b.x < -eps || b.y < -eps) {
            r.ok = false;
            r.errors.push_back("negative position: " + b.name);
        }
    }

    // 2. Check every pair of blocks once (i < j) for overlap.
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            const Block& a = d.blocks[i];
            const Block& b = d.blocks[j];

            // Two rectangles are apart if one is completely left/right of the
            // other, or completely below/above. Touching edges counts as apart.
            bool apartInX = (a.x + a.w() <= b.x + eps) || (b.x + b.w() <= a.x + eps);
            bool apartInY = (a.y + a.h() <= b.y + eps) || (b.y + b.h() <= a.y + eps);

            if (!apartInX && !apartInY) {
                r.ok = false;
                r.errors.push_back("overlap: " + a.name + " and " + b.name);
            }
        }
    }

    return r;
}

}  // namespace tp
