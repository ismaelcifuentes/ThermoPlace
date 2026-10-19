// ThermoPlace - CSV reports.
// OWNER: Eimi. Follow docs/SEMANA2.md. Contract: include/thermoplace/Report.hpp
#include "thermoplace/Report.hpp"

namespace tp {

bool writeTraceCSV(const std::vector<SATracePoint>& trace, const std::string& path,
                   std::string& err) {
    // TODO(Eimi): task E9.
    (void)trace; (void)path;
    err = "writeTraceCSV not implemented yet (TODO Eimi)";
    return false;
}

bool appendRunCSV(const RunRecord& r, const std::string& path, std::string& err) {
    // TODO(Eimi): task E9.
    (void)r; (void)path;
    err = "appendRunCSV not implemented yet (TODO Eimi)";
    return false;
}

}  // namespace tp
