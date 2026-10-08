// ThermoPlace - floorplan metrics.
// OWNER: Jose. Follow docs/MANUAL_JOSE.md. Contract: include/thermoplace/Metrics.hpp
#include "thermoplace/Metrics.hpp"

namespace tp {

Metrics computeMetrics(const Design& d) {
    // TODO(Jose): task J1.
    (void)d;
    return Metrics{};
}

double computeHPWL(const Design& d) {
    // TODO(Jose): task J2.
    (void)d;
    return -1.0;  // wrong on purpose so the tests fail until implemented
}

}  // namespace tp
