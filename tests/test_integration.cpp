// End-to-end tests: need everybody's part. Run: thermoplace_tests integration
#include "test_framework.hpp"
#include "thermoplace/BStarTree.hpp"
#include "thermoplace/Metrics.hpp"
#include "thermoplace/Parser.hpp"
#include "thermoplace/Validator.hpp"

using namespace tp;

TEST(integration, all_mcnc_benchmarks_pack_legally) {
    const char* names[] = {"ami33", "ami49", "apte", "hp", "xerox"};
    for (const char* n : names) {
        Design d;
        std::string err;
        REQUIRE(loadBenchmark(tt::bench(std::string("mcnc/") + n + ".block"), d, err));
        BStarTree t;
        t.buildInitial(static_cast<int>(d.blocks.size()));
        t.pack(d);
        ValidationReport r = validatePlacement(d);
        CHECK(r.ok);
        Metrics m = computeMetrics(d);
        CHECK(m.chipArea >= m.blockArea);
        CHECK(m.deadSpacePct >= 0 && m.deadSpacePct < 100);
        CHECK(m.hpwl > 0);
    }
}
