// Tests for Jose's module (Metrics, Validator, Exporters, Power). Run: thermoplace_tests jose
#include <fstream>
#include <sstream>

#include "test_framework.hpp"
#include "thermoplace/Exporters.hpp"
#include "thermoplace/Metrics.hpp"
#include "thermoplace/Power.hpp"
#include "thermoplace/Validator.hpp"

using namespace tp;

namespace {
// A, B, C already placed (no B*-tree needed): chip 6 x 3.
Design placedTiny() {
    Design d;
    d.name = "tiny";
    d.addBlock("A", 4, 2); d.blocks[0].x = 0; d.blocks[0].y = 0;
    d.addBlock("B", 2, 2); d.blocks[1].x = 4; d.blocks[1].y = 0;
    d.addBlock("C", 4, 1); d.blocks[2].x = 0; d.blocks[2].y = 2;
    d.addTerminal("T", 10, 10);
    Net n1; n1.blocks = {0, 1};              d.nets.push_back(n1);
    Net n2; n2.blocks = {0, 2}; n2.terminals = {0}; d.nets.push_back(n2);
    Net n3; n3.blocks = {1};                 d.nets.push_back(n3);  // 1 pin -> 0
    return d;
}

std::vector<std::string> readLines(const std::string& path) {
    std::ifstream in(path);
    std::vector<std::string> out;
    std::string l;
    while (std::getline(in, l)) out.push_back(l);
    return out;
}
}  // namespace

TEST(metrics, bounding_box_and_dead_space) {
    Design d = placedTiny();
    Metrics m = computeMetrics(d);
    CHECK_NEAR(m.chipW, 6, 1e-9);
    CHECK_NEAR(m.chipH, 3, 1e-9);
    CHECK_NEAR(m.chipArea, 18, 1e-9);
    CHECK_NEAR(m.blockArea, 16, 1e-9);
    CHECK_NEAR(m.deadSpacePct, 100.0 * 2.0 / 18.0, 1e-9);
    CHECK(m.fitsOutline);  // no outline given
}

TEST(metrics, outline_check) {
    Design d = placedTiny();
    d.outlineW = 5; d.outlineH = 5;
    CHECK(!computeMetrics(d).fitsOutline);  // width 6 > 5
    d.outlineW = 6;
    CHECK(computeMetrics(d).fitsOutline);
}

TEST(metrics, rotation_changes_bounding_box) {
    Design d = placedTiny();
    d.blocks[1].x = 4; d.blocks[1].rotated = true;  // 2x2 -> still 2x2
    d.blocks[2].rotated = true;                    // C: 1 wide, 4 tall at (0,2)
    Metrics m = computeMetrics(d);
    CHECK_NEAR(m.chipH, 6, 1e-9);
}

TEST(metrics, hpwl) {
    Design d = placedTiny();
    // net1: A(2,1) B(5,1) -> 3 ; net2: A(2,1) C(2,2.5) T(10,10) -> 8 + 9 = 17 ; net3: 0
    CHECK_NEAR(computeHPWL(d), 20, 1e-9);
    CHECK_NEAR(computeMetrics(d).hpwl, 20, 1e-9);
}

TEST(validator, legal_placement_ok) {
    ValidationReport r = validatePlacement(placedTiny());
    CHECK(r.ok);
    CHECK(r.errors.empty());
}

TEST(validator, detects_overlap) {
    Design d = placedTiny();
    d.blocks[1].x = 3;  // B now overlaps A
    ValidationReport r = validatePlacement(d);
    CHECK(!r.ok);
    REQUIRE(!r.errors.empty());
    CHECK(r.errors[0].find("A") != std::string::npos);
    CHECK(r.errors[0].find("B") != std::string::npos);
}

TEST(validator, detects_negative_and_empty) {
    Design d = placedTiny();
    d.blocks[2].y = -1;
    CHECK(!validatePlacement(d).ok);
    Design e = placedTiny();
    e.blocks[0].width = 0;
    CHECK(!validatePlacement(e).ok);
}

TEST(exporters, svg) {
    Design d = placedTiny();
    std::string err;
    REQUIRE(exportSVG(d, tt::out("test_tiny.svg"), err));
    std::ifstream in(tt::out("test_tiny.svg"));
    std::stringstream ss; ss << in.rdbuf();
    std::string s = ss.str();
    CHECK(s.find("<svg") != std::string::npos);
    CHECK(s.find("</svg>") != std::string::npos);
    CHECK(s.find(">A<") != std::string::npos);  // block labels as <text>..</text>
    CHECK(s.find(">C<") != std::string::npos);
    std::size_t rects = 0, pos = 0;
    while ((pos = s.find("<rect", pos)) != std::string::npos) { ++rects; ++pos; }
    CHECK(rects >= 3);
}

TEST(exporters, hotspot_flp) {
    Design d = placedTiny();
    std::string err;
    REQUIRE(exportHotSpotFLP(d, tt::out("test_tiny.flp"), 1e-6, err));
    int dataLines = 0;
    for (const std::string& l : readLines(tt::out("test_tiny.flp"))) {
        if (l.empty() || l[0] == '#') continue;
        ++dataLines;
        std::istringstream in(l);
        std::string name; double w, h, x, y;
        REQUIRE(static_cast<bool>(in >> name >> w >> h >> x >> y));
        if (name == "A") { CHECK_NEAR(w, 4e-6, 1e-15); CHECK_NEAR(h, 2e-6, 1e-15); }
        if (name == "C") { CHECK_NEAR(y, 2e-6, 1e-15); }
        CHECK(l.find('\t') != std::string::npos);
    }
    CHECK_EQ(dataLines, 3);
}

TEST(exporters, bad_path_fails) {
    std::string err;
    CHECK(!exportSVG(placedTiny(), "no_such_dir_xyz/out.svg", err));
    CHECK(!err.empty());
}

// ---------------- EXTRA (week 5) ----------------
TEST(exporters_extra, csv_and_ptrace) {
    Design d = placedTiny();
    std::string err;
    REQUIRE(exportCSV(d, tt::out("test_tiny.csv"), err));
    CHECK_EQ(readLines(tt::out("test_tiny.csv")).size(), 4u);  // header + 3
    REQUIRE(exportPTrace(d, tt::out("test_tiny.ptrace"), err));
    std::vector<std::string> p = readLines(tt::out("test_tiny.ptrace"));
    REQUIRE(p.size() >= 2u);
    CHECK(p[0].find("A\tB\tC") != std::string::npos);
}

TEST(power_extra, reproducible_synthetic_power) {
    Design a = placedTiny(), b = placedTiny(), c = placedTiny();
    assignSyntheticPower(a, 7, 0.1, 0.5);
    assignSyntheticPower(b, 7, 0.1, 0.5);
    assignSyntheticPower(c, 8, 0.1, 0.5);
    for (std::size_t i = 0; i < a.blocks.size(); ++i) {
        CHECK_NEAR(a.blocks[i].power, b.blocks[i].power, 1e-12);
        CHECK(a.blocks[i].power >= 0.1 * a.blocks[i].area() - 1e-12);
        CHECK(a.blocks[i].power <= 0.5 * a.blocks[i].area() + 1e-12);
    }
    CHECK(a.blocks[0].power != c.blocks[0].power);
    CHECK(totalPower(a) > 0);
}
