// ThermoPlace - cost function used by simulated annealing.
// OWNER: Jose  (implementation goes in src/Cost.cpp)
//
//   cost = alpha * area / norm.area  +  (1 - alpha) * hpwl / norm.hpwl
//          + outline * outlinePenalty
//          (+ thermal * Tmax / norm.temp, week 5)
//
// Area and HPWL have very different units, so each one is divided by a
// reference value (normally the numbers of the initial floorplan): then both
// terms are close to 1 and `alpha` really sets their relative weight.
#pragma once

#include "thermoplace/Design.hpp"
#include "thermoplace/Metrics.hpp"

namespace tp {

struct CostWeights {
    double alpha = 0.5;    // 1 = only area, 0 = only wirelength
    double outline = 0.0;  // weight of the fixed-outline penalty (0 = ignore outline)
    double thermal = 0.0;  // week 5: weight of the peak temperature (unused for now)
};

struct CostNorm {
    double area = 1.0;
    double hpwl = 1.0;
};

// Reference values taken from a floorplan (usually the initial one).
// A value <= 0 is replaced by 1 so we never divide by zero.
CostNorm makeNorm(const Metrics& m);

// 0 if the design has no outline (outlineW or outlineH <= 0) or the chip fits.
// Otherwise (exW / outlineW)^2 + (exH / outlineH)^2, where
// exW = max(0, chipW - outlineW) and exH = max(0, chipH - outlineH).
double outlinePenalty(const Metrics& m, const Design& d);

// The formula at the top of this file (thermal term ignored until week 5).
double computeCost(const Metrics& m, const Design& d, const CostWeights& w,
                   const CostNorm& norm);

}  // namespace tp
