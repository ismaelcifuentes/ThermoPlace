// ThermoPlace - synthetic power assignment.
// OWNER: Jose  (EXTRA; implementation goes in src/Power.cpp)
//
// The MCNC and GSRC benchmarks only give block sizes, not power. Thermal-aware
// floorplanning papers assign power densities themselves; we do it with a
// FIXED SEED so every experiment is reproducible (write the seed and the
// density range in the paper).
#pragma once

#include "thermoplace/Design.hpp"

namespace tp {

// For each block: density ~ Uniform[minDensity, maxDensity] (W per unit^2),
// power = density * area. Same seed -> same powers, always.
void assignSyntheticPower(Design& d, unsigned seed, double minDensity, double maxDensity);

// Sum of Block::power.
double totalPower(const Design& d);

}  // namespace tp
