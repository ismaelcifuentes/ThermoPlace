// ThermoPlace - synthetic, reproducible power assignment.
// OWNER: Jose (EXTRA). Follow docs/MANUAL_JOSE.md. Contract: include/thermoplace/Power.hpp
#include "thermoplace/Power.hpp"

#include <random>

namespace tp {

void assignSyntheticPower(Design& d, unsigned seed, double minDensity, double maxDensity) {
    // Same seed -> same sequence of random numbers -> same powers every run.
    std::mt19937 rng(seed);
    std::uniform_real_distribution<double> density(minDensity, maxDensity);

    // Each block gets its own random density (W per unit^2), times its area.
    for (Block& b : d.blocks) {
        b.power = density(rng) * b.area();
    }
}

double totalPower(const Design& d) {
    double p = 0.0;
    for (const Block& b : d.blocks) p += b.power;
    return p;
}

}  // namespace tp
