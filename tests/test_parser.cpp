// Tests for Eimi's module (Parser). Run: thermoplace_tests eimi
#include "test_framework.hpp"
#include "thermoplace/Parser.hpp"

using namespace tp;

TEST(parser, tiny_blocks) {
    Design d;
    std::string err;
    REQUIRE(parseMcncBlocks(tt::bench("tiny/tiny3.block"), d, err));
    CHECK_EQ(d.blocks.size(), 3u);
    CHECK_EQ(d.terminals.size(), 1u);
    CHECK_NEAR(d.outlineW, 10, 1e-9);
    CHECK_NEAR(d.outlineH, 10, 1e-9);
    REQUIRE(d.findBlock("C") == 2);
    CHECK_NEAR(d.blocks[2].width, 4, 1e-9);
    CHECK_NEAR(d.blocks[2].height, 1, 1e-9);
    REQUIRE(d.findTerminal("P1") == 0);
    CHECK_NEAR(d.terminals[0].x, 10, 1e-9);
}

TEST(parser, ami33_blocks_and_crlf) {
    // ami33.block has Windows line endings (\r\n): names must not keep a '\r'.
    Design d;
    std::string err;
    REQUIRE(parseMcncBlocks(tt::bench("mcnc/ami33.block"), d, err));
    CHECK_EQ(d.blocks.size(), 33u);
    CHECK_EQ(d.terminals.size(), 40u);
    CHECK_NEAR(d.outlineW, 1326, 1e-9);
    CHECK_NEAR(d.outlineH, 1205, 1e-9);
    REQUIRE(d.findBlock("bk1") == 0);   // fails if the name is "bk1\r"
    CHECK_NEAR(d.blocks[0].width, 336, 1e-9);
    CHECK_NEAR(d.blocks[0].height, 133, 1e-9);
    CHECK_NEAR(d.totalBlockArea(), 1156449, 1e-6);  // known ami33 area
    int vss = d.findTerminal("VSS");
    REQUIRE(vss >= 0);
    CHECK_NEAR(d.terminals[vss].x, 1410, 1e-9);
    CHECK_NEAR(d.terminals[vss].y, 1610, 1e-9);
}

TEST(parser, tiny_nets) {
    Design d;
    std::string err;
    REQUIRE(parseMcncBlocks(tt::bench("tiny/tiny3.block"), d, err));
    REQUIRE(parseNets(tt::bench("tiny/tiny3.nets"), d, err));
    REQUIRE(d.nets.size() == 2u);
    CHECK_EQ(d.nets[0].blocks.size(), 2u);
    CHECK_EQ(d.nets[0].terminals.size(), 0u);
    CHECK_EQ(d.nets[1].blocks.size(), 2u);
    CHECK_EQ(d.nets[1].terminals.size(), 1u);
}

TEST(parser, ami33_nets) {
    Design d;
    std::string err;
    REQUIRE(parseMcncBlocks(tt::bench("mcnc/ami33.block"), d, err));
    REQUIRE(parseNets(tt::bench("mcnc/ami33.nets"), d, err));
    REQUIRE(d.nets.size() == 121u);
    // First net: GND (terminal) + all 33 blocks = degree 34.
    CHECK_EQ(d.nets[0].blocks.size() + d.nets[0].terminals.size(), 34u);
    std::size_t pins = 0;
    for (const Net& n : d.nets) pins += n.blocks.size() + n.terminals.size();
    CHECK_EQ(pins, 425u);
}

TEST(parser, load_benchmark_mcnc) {
    Design d;
    std::string err;
    REQUIRE(loadBenchmark(tt::bench("mcnc/ami33.block"), d, err));
    CHECK_EQ(d.name, std::string("ami33"));
    CHECK_EQ(d.blocks.size(), 33u);
    CHECK_EQ(d.nets.size(), 121u);
}

TEST(parser, missing_file_reports_error) {
    Design d;
    std::string err;
    CHECK(!loadBenchmark(tt::bench("mcnc/does_not_exist.block"), d, err));
    CHECK(!err.empty());
}

// ---------------- EXTRA (GSRC, week 7) ----------------
TEST(parser_extra, gsrc_n100) {
    Design d;
    std::string err;
    REQUIRE(loadBenchmark(tt::bench("gsrc/n100.hardblocks"), d, err));
    CHECK_EQ(d.name, std::string("n100"));
    CHECK_EQ(d.blocks.size(), 100u);
    CHECK_EQ(d.terminals.size(), 334u);
    CHECK_EQ(d.nets.size(), 885u);
    REQUIRE(d.findBlock("sb0") == 0);
    CHECK_NEAR(d.blocks[0].width, 43, 1e-9);   // (43, 33) corner
    CHECK_NEAR(d.blocks[0].height, 33, 1e-9);
    int p2 = d.findTerminal("p2");
    REQUIRE(p2 >= 0);
    CHECK_NEAR(d.terminals[p2].x, 4, 1e-9);    // from n100.pl
    CHECK_NEAR(d.terminals[p2].y, 0, 1e-9);
}
