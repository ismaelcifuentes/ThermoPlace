// ThermoPlace - CSV reports.
// OWNER: Eimi. Follow docs/SEMANA2.md. Contract: include/thermoplace/Report.hpp
#include "thermoplace/Report.hpp"

#include <fstream>
#include <iomanip>

namespace tp {

namespace {

// The default precision (6) would write ami33's chip area as 4.67578e+06, which
// loses digits and some spreadsheets read as text. 10 significant digits keep
// areas exact and costs (around 1.0) readable.
void numberFormat(std::ostream& out) {
    out << std::setprecision(10);
}

}  // namespace

bool writeTraceCSV(const std::vector<SATracePoint>& trace, const std::string& path,
                   std::string& err) {
    std::ofstream out(path);  // truncates: every run starts its trace from scratch
    if (!out) {
        err = "cannot write " + path;
        return false;
    }
    numberFormat(out);

    out << "step,temperature,current_cost,best_cost,accept_rate\n";
    for (const SATracePoint& p : trace) {
        out << p.step << ',' << p.temperature << ',' << p.currentCost << ','
            << p.bestCost << ',' << p.acceptRate << '\n';
    }
    return true;
}

bool appendRunCSV(const RunRecord& r, const std::string& path, std::string& err) {
    // A missing file and an empty one both need the header. peek() on a stream that
    // failed to open also returns EOF, so one check covers both cases without
    // std::filesystem (it breaks the MinGW bundled with Code::Blocks).
    bool needsHeader;
    {
        std::ifstream probe(path);
        needsHeader = probe.peek() == std::ifstream::traits_type::eof();
    }

    std::ofstream out(path, std::ios::app);
    if (!out) {
        err = "cannot write " + path;
        return false;
    }
    numberFormat(out);

    if (needsHeader) {
        out << "benchmark,method,seed,alpha,outline_weight,chip_area,dead_space_pct,"
               "hpwl,fits_outline,seconds\n";
    }
    out << r.benchmark << ',' << r.method << ',' << r.seed << ',' << r.alpha << ','
        << r.outlineWeight << ',' << r.chipArea << ',' << r.deadSpacePct << ',' << r.hpwl
        << ',' << (r.fitsOutline ? 1 : 0) << ',' << r.seconds << '\n';
    return true;
}

}  // namespace tp
