// ThermoPlace - CSV reports of the experiments (one file per run + one table of runs).
// OWNER: Eimi  (implementation goes in src/Report.cpp)
//
// The paper needs numbers from many runs (different seeds, alphas, benchmarks).
// These two functions write them as CSV, which opens in Excel / Google Sheets.
#pragma once

#include <string>
#include <vector>

#include "thermoplace/Annealer.hpp"

namespace tp {

// Writes the SA trace, overwriting the file. Exact header:
//   step,temperature,current_cost,best_cost,accept_rate
// then one line per SATracePoint, in order. Returns false (and sets err) if the
// file cannot be opened.
bool writeTraceCSV(const std::vector<SATracePoint>& trace, const std::string& path,
                   std::string& err);

// One finished run, for the results table of the paper.
struct RunRecord {
    std::string benchmark;    // e.g. "ami33"
    std::string method;       // "random" or "sa"
    unsigned seed = 1;
    double alpha = 0.5;
    double outlineWeight = 0.0;
    double chipArea = 0.0;
    double deadSpacePct = 0.0;
    double hpwl = 0.0;
    bool fitsOutline = false;
    double seconds = 0.0;     // run time
};

// Appends one line to `path`. If the file does not exist yet or is empty, it
// first writes the exact header:
//   benchmark,method,seed,alpha,outline_weight,chip_area,dead_space_pct,hpwl,fits_outline,seconds
// fits_outline is written as 1 or 0.
bool appendRunCSV(const RunRecord& r, const std::string& path, std::string& err);

}  // namespace tp
