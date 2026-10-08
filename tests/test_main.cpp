// Test runner.
//   thermoplace_tests            -> runs everything
//   thermoplace_tests ismael     -> only Ismael's groups (contour, bstar)
//   thermoplace_tests eimi       -> only Eimi's groups   (parser)
//   thermoplace_tests jose       -> only Jose's groups   (metrics, validator, exporters)
//   thermoplace_tests extra      -> optional groups      (*_extra)
//   thermoplace_tests <group>    -> one group, e.g. "contour"
#include <map>
#include <set>

#include "test_framework.hpp"

int main(int argc, char** argv) {
    std::map<std::string, std::set<std::string>> owners = {
        {"ismael", {"contour", "bstar"}},
        {"eimi", {"parser"}},
        {"jose", {"metrics", "validator", "exporters"}},
        {"extra", {"bstar_extra", "parser_extra", "exporters_extra", "power_extra"}},
        {"core", {"contour", "bstar", "parser", "metrics", "validator", "exporters", "integration"}},
    };
    std::set<std::string> wanted;
    if (argc > 1) {
        std::string f = argv[1];
        if (owners.count(f)) wanted = owners[f];
        else wanted.insert(f);
    }

    std::map<std::string, std::pair<int, int>> perGroup;  // group -> (passed, total)
    int passed = 0, total = 0;
    for (const tt::TestCase& t : tt::registry()) {
        if (!wanted.empty() && !wanted.count(t.group)) continue;
        tt::checksFailedInTest() = 0;
        std::cout << "[" << t.group << "] " << t.name << std::endl;  // flush: see the name even if it crashes
        try {
            t.fn();
        } catch (const tt::AbortTest&) {
        } catch (const std::exception& e) {
            tt::reportFailure("exception", 0, e.what());
        }
        bool ok = tt::checksFailedInTest() == 0;
        std::cout << (ok ? "      ok\n" : "");
        ++total;
        ++perGroup[t.group].second;
        if (ok) { ++passed; ++perGroup[t.group].first; }
    }

    std::cout << "\n========== SUMMARY ==========\n";
    for (const auto& g : perGroup)
        std::cout << "  " << g.first << ": " << g.second.first << "/" << g.second.second
                  << (g.second.first == g.second.second ? "  PASS" : "  FAIL") << "\n";
    std::cout << "  TOTAL: " << passed << "/" << total << "\n";
    return passed == total ? 0 : 1;
}
