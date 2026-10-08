// ThermoPlace - SVG / HotSpot / CSV writers.
// OWNER: Jose. Follow docs/MANUAL_JOSE.md. Contract: include/thermoplace/Exporters.hpp
#include "thermoplace/Exporters.hpp"

#include <fstream>

#include "thermoplace/Metrics.hpp"

namespace tp {

bool exportSVG(const Design& d, const std::string& path, std::string& err) {
    // TODO(Jose): task J4.
    (void)d; (void)path;
    err = "exportSVG not implemented yet (TODO Jose)";
    return false;
}

bool exportHotSpotFLP(const Design& d, const std::string& path, double unitToMeters,
                      std::string& err) {
    // TODO(Jose): task J5.
    (void)d; (void)path; (void)unitToMeters;
    err = "exportHotSpotFLP not implemented yet (TODO Jose)";
    return false;
}

bool exportPTrace(const Design& d, const std::string& path, std::string& err) {
    // TODO(Jose, EXTRA): task J7.
    (void)d; (void)path;
    err = "exportPTrace not implemented yet (TODO Jose, extra)";
    return false;
}

bool exportCSV(const Design& d, const std::string& path, std::string& err) {
    // TODO(Jose, EXTRA): task J6.
    (void)d; (void)path;
    err = "exportCSV not implemented yet (TODO Jose, extra)";
    return false;
}

}  // namespace tp
