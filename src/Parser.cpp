// ThermoPlace - benchmark readers.
// OWNER: Eimi. Follow docs/MANUAL_EIMI.md. Contract: include/thermoplace/Parser.hpp
#include "thermoplace/Parser.hpp"

#include <fstream>

#include "thermoplace/StrUtil.hpp"

namespace tp {

bool parseMcncBlocks(const std::string& path, Design& d, std::string& err) {
    // TODO(Eimi): read "Outline:", "NumBlocks:", "NumTerminals:", block lines
    //             and terminal lines. See MANUAL_EIMI.md, task E2.
    (void)path; (void)d;
    err = "parseMcncBlocks not implemented yet (TODO Eimi)";
    return false;
}

bool parseNets(const std::string& path, Design& d, std::string& err) {
    // TODO(Eimi): read "NetDegree : k" + k pin names per net. Task E3.
    (void)path; (void)d;
    err = "parseNets not implemented yet (TODO Eimi)";
    return false;
}

bool parseGsrcHardBlocks(const std::string& path, Design& d, std::string& err) {
    // TODO(Eimi, EXTRA): task E6.
    (void)path; (void)d;
    err = "parseGsrcHardBlocks not implemented yet (TODO Eimi, extra)";
    return false;
}

bool parseGsrcPl(const std::string& path, Design& d, std::string& err) {
    // TODO(Eimi, EXTRA): task E6.
    (void)path; (void)d;
    err = "parseGsrcPl not implemented yet (TODO Eimi, extra)";
    return false;
}

bool loadBenchmark(const std::string& path, Design& d, std::string& err) {
    // TODO(Eimi): detect ".block" / ".hardblocks", load siblings, set d.name. Task E4.
    (void)path; (void)d;
    err = "loadBenchmark not implemented yet (TODO Eimi)";
    return false;
}

}  // namespace tp
