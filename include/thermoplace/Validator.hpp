// ThermoPlace - checks that a packed floorplan is legal.
// OWNER: Jose  (implementation goes in src/Validator.cpp)
#pragma once

#include <string>
#include <vector>

#include "thermoplace/Design.hpp"

namespace tp {

struct ValidationReport {
    bool ok = true;
    std::vector<std::string> errors;  // one message per problem found
};

// A placement is legal when:
//   1. every block has x >= 0 and y >= 0,
//   2. no two blocks overlap (touching edges is allowed; use tolerance eps),
//   3. every block has positive width and height.
// Each problem adds a message such as "overlap: bk1 and bk7".
ValidationReport validatePlacement(const Design& d, double eps = 1e-6);

}  // namespace tp
