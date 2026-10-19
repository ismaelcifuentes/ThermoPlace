// ThermoPlace - cost function.
// OWNER: Jose. Follow docs/SEMANA2.md. Contract: include/thermoplace/Cost.hpp
#include "thermoplace/Cost.hpp"

namespace tp {

CostNorm makeNorm(const Metrics& m) {
    // TODO(Jose): task J8.
    (void)m;
    return CostNorm{-1.0, -1.0};
}

double outlinePenalty(const Metrics& m, const Design& d) {
    // TODO(Jose): task J8.
    (void)m; (void)d;
    return -1.0;
}

double computeCost(const Metrics& m, const Design& d, const CostWeights& w,
                   const CostNorm& norm) {
    // TODO(Jose): task J8.
    (void)m; (void)d; (void)w; (void)norm;
    return -1.0;
}

}  // namespace tp
