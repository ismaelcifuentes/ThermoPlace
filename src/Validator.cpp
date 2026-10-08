// ThermoPlace - placement legality checks.
// OWNER: Jose. Follow docs/MANUAL_JOSE.md. Contract: include/thermoplace/Validator.hpp
#include "thermoplace/Validator.hpp"

namespace tp {

ValidationReport validatePlacement(const Design& d, double eps) {
    // TODO(Jose): task J3.
    (void)d; (void)eps;
    ValidationReport r;
    r.ok = false;
    r.errors.push_back("validatePlacement not implemented yet (TODO Jose)");
    return r;
}

}  // namespace tp
