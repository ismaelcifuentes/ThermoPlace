// ThermoPlace - cost function.
// OWNER: Jose. Follow docs/SEMANA2.md. Contract: include/thermoplace/Cost.hpp
#include "thermoplace/Cost.hpp"

#include <algorithm>  // std::max

namespace tp {

CostNorm makeNorm(const Metrics& m) {
    // The reference values are just the area and HPWL of the given floorplan.
    // If one of them is 0 (or negative) we use 1 so the cost never divides by zero.
    CostNorm n;
    n.area = (m.chipArea > 0) ? m.chipArea : 1.0;
    n.hpwl = (m.hpwl > 0) ? m.hpwl : 1.0;
    return n;
}

double outlinePenalty(const Metrics& m, const Design& d) {
    // No fixed outline in the benchmark: nothing to penalize.
    if (d.outlineW <= 0 || d.outlineH <= 0) return 0.0;

    // How much the chip sticks out of the outline in each direction (0 if it fits).
    double exW = std::max(0.0, m.chipW - d.outlineW);
    double exH = std::max(0.0, m.chipH - d.outlineH);

    // Relative excess, squared: small overflows cost little, big ones a lot.
    double rw = exW / d.outlineW;
    double rh = exH / d.outlineH;
    return rw * rw + rh * rh;
}

double computeCost(const Metrics& m, const Design& d, const CostWeights& w,
                   const CostNorm& norm) {
    // Each term is divided by its reference value, so both are close to 1
    // and alpha really decides how much area matters versus wirelength.
    double areaTerm = m.chipArea / norm.area;
    double wireTerm = m.hpwl / norm.hpwl;
    double cost = w.alpha * areaTerm + (1.0 - w.alpha) * wireTerm;

    // Extra penalty when the chip does not fit the fixed outline.
    cost += w.outline * outlinePenalty(m, d);

    // w.thermal is used in week 5 (peak temperature); ignored for now.
    return cost;
}

}  // namespace tp
