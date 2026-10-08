// ThermoPlace - synthetic, reproducible power assignment.
// OWNER: Jose (EXTRA). Follow docs/MANUAL_JOSE.md. Contract: include/thermoplace/Power.hpp
#include "thermoplace/Power.hpp"

#include <random>

namespace tp {

void assignSyntheticPower(Design& d, unsigned seed, double minDensity, double maxDensity) {
    // TODO(Jose, EXTRA): task J7. Use std::mt19937 with `seed` and
    // std::uniform_real_distribution<double>(minDensity, maxDensity).
    (void)d; (void)seed; (void)minDensity; (void)maxDensity;
}

double totalPower(const Design& d) {
    double p = 0.0;
    for (const Block& b : d.blocks) p += b.power;
    return p;
}

}  // namespace tp
