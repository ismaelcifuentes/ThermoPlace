// Tests for Ismael's module (Contour + BStarTree). Run: thermoplace_tests ismael
#include <random>

#include "test_framework.hpp"
#include "thermoplace/BStarTree.hpp"
#include "thermoplace/Contour.hpp"

using namespace tp;

namespace {
// Independent overlap check so these tests do not depend on Jose's Validator.
bool noOverlaps(const Design& d) {
    const double eps = 1e-9;
    for (std::size_t i = 0; i < d.blocks.size(); ++i) {
        const Block& a = d.blocks[i];
        if (a.x < -eps || a.y < -eps) return false;
        for (std::size_t j = i + 1; j < d.blocks.size(); ++j) {
            const Block& b = d.blocks[j];
            bool sepX = a.x + a.w() <= b.x + eps || b.x + b.w() <= a.x + eps;
            bool sepY = a.y + a.h() <= b.y + eps || b.y + b.h() <= a.y + eps;
            if (!sepX && !sepY) return false;
        }
    }
    return true;
}
}  // namespace

TEST(contour, empty_is_zero) {
    Contour c;
    c.clear();
    CHECK_NEAR(c.maxHeight(0, 10), 0, 1e-12);
}

TEST(contour, insert_and_query) {
    Contour c;
    c.clear();
    c.insert(0, 4, 2);
    CHECK_NEAR(c.maxHeight(0, 4), 2, 1e-12);
    CHECK_NEAR(c.maxHeight(3, 5), 2, 1e-12);
    CHECK_NEAR(c.maxHeight(4, 6), 0, 1e-12);  // only touches at x=4 -> no overlap
    c.insert(4, 6, 5);
    CHECK_NEAR(c.maxHeight(0, 10), 5, 1e-12);
    CHECK_NEAR(c.maxHeight(0, 4), 2, 1e-12);
    c.clear();
    CHECK_NEAR(c.maxHeight(0, 10), 0, 1e-12);
}

TEST(bstar, build_initial_shape) {
    BStarTree t;
    t.buildInitial(5);
    REQUIRE(t.size() == 5);
    CHECK_EQ(t.root(), 0);
    CHECK_EQ(t.node(0).parent, -1);
    CHECK_EQ(t.node(0).left, 1);
    CHECK_EQ(t.node(0).right, 2);
    CHECK_EQ(t.node(1).left, 3);
    CHECK_EQ(t.node(1).right, 4);
    CHECK_EQ(t.node(2).left, -1);
    CHECK_EQ(t.node(3).parent, 1);
    for (int i = 0; i < 5; ++i) CHECK_EQ(t.node(i).block, i);
    CHECK(t.isValid());
}

TEST(bstar, build_initial_edge_cases) {
    BStarTree t;
    t.buildInitial(0);
    CHECK_EQ(t.size(), 0);
    CHECK_EQ(t.root(), -1);
    CHECK(t.isValid());  // an empty tree is valid
    t.buildInitial(1);
    REQUIRE(t.size() == 1);
    CHECK_EQ(t.root(), 0);
    CHECK_EQ(t.node(0).left, -1);
    CHECK_EQ(t.node(0).right, -1);
    CHECK(t.isValid());
}

TEST(bstar, pack_three_blocks) {
    // Tree: 0 = A (root), left 1 = B (to the right of A), right 2 = C (above A)
    Design d;
    d.addBlock("A", 4, 2);
    d.addBlock("B", 2, 2);
    d.addBlock("C", 4, 1);
    BStarTree t;
    t.buildInitial(3);
    t.pack(d);
    CHECK_NEAR(d.blocks[0].x, 0, 1e-9); CHECK_NEAR(d.blocks[0].y, 0, 1e-9);
    CHECK_NEAR(d.blocks[1].x, 4, 1e-9); CHECK_NEAR(d.blocks[1].y, 0, 1e-9);
    CHECK_NEAR(d.blocks[2].x, 0, 1e-9); CHECK_NEAR(d.blocks[2].y, 2, 1e-9);
}

TEST(bstar, pack_uses_contour_of_neighbours) {
    // node0=b0(2x2) left->node1=b1(2x5) left->node3=b3(1x1); node0 right->node2=b2(3x1)
    // b2 sits above b0 at x=0 but it is 3 wide, so it also covers x in [2,3)
    // where b1 is 5 tall -> b2.y must be 5, not 2.
    Design d;
    d.addBlock("b0", 2, 2);
    d.addBlock("b1", 2, 5);
    d.addBlock("b2", 3, 1);
    d.addBlock("b3", 1, 1);
    BStarTree t;
    t.buildInitial(4);
    t.pack(d);
    CHECK_NEAR(d.blocks[1].x, 2, 1e-9); CHECK_NEAR(d.blocks[1].y, 0, 1e-9);
    CHECK_NEAR(d.blocks[3].x, 4, 1e-9); CHECK_NEAR(d.blocks[3].y, 0, 1e-9);
    CHECK_NEAR(d.blocks[2].x, 0, 1e-9); CHECK_NEAR(d.blocks[2].y, 5, 1e-9);
    CHECK(noOverlaps(d));
}

TEST(bstar, pack_respects_rotation) {
    Design d;
    d.addBlock("A", 4, 2);
    d.addBlock("B", 3, 1);
    d.addBlock("C", 4, 1);
    d.blocks[0].rotated = true;  // A becomes 2 wide, 4 tall
    BStarTree t;
    t.buildInitial(3);
    t.pack(d);
    CHECK_NEAR(d.blocks[1].x, 2, 1e-9);  // right of rotated A
    CHECK_NEAR(d.blocks[2].y, 4, 1e-9);  // above rotated A
    CHECK(noOverlaps(d));
}

TEST(bstar, swap_blocks) {
    BStarTree t;
    t.buildInitial(3);
    REQUIRE(t.size() == 3);
    t.swapBlocks(1, 2);
    CHECK_EQ(t.node(1).block, 2);
    CHECK_EQ(t.node(2).block, 1);
    CHECK_EQ(t.node(0).left, 1);  // shape unchanged
    CHECK(t.isValid());
}

TEST(bstar, random_packings_never_overlap) {
    std::mt19937 rng(42);
    std::uniform_real_distribution<double> size(1.0, 50.0);
    Design d;
    for (int i = 0; i < 60; ++i) d.addBlock("r" + std::to_string(i), size(rng), size(rng));
    BStarTree t;
    t.buildInitial(60);
    REQUIRE(t.size() == 60);
    std::uniform_int_distribution<int> pick(0, 59);
    for (int it = 0; it < 200; ++it) {
        t.swapBlocks(pick(rng), pick(rng));
        d.blocks[pick(rng)].rotated ^= true;
        t.pack(d);
        REQUIRE(noOverlaps(d));
    }
    CHECK(t.isValid());
}

TEST(bstar, to_string_lists_nodes) {
    BStarTree t;
    t.buildInitial(2);
    std::string s = t.toString();
    CHECK(s.find("node 0") != std::string::npos);
    CHECK(s.find("node 1") != std::string::npos);
}

// ---------------- EXTRA (week 3) ----------------
TEST(bstar_extra, move_leaf) {
    BStarTree t;
    t.buildInitial(5);              // 0(l1,r2) 1(l3,r4) 2 3 4 leaves
    REQUIRE(t.size() == 5);
    REQUIRE(t.moveNode(4, 2, true));  // leaf 4 -> left child of 2
    CHECK_EQ(t.node(4).parent, 2);
    CHECK_EQ(t.node(2).left, 4);
    CHECK_EQ(t.node(1).right, -1);
    CHECK(t.isValid());
    CHECK(!t.moveNode(3, 2, true));  // slot already used
    CHECK(!t.moveNode(1, 2, false)); // node 1 is not a leaf (simple version)
    CHECK(t.isValid());
}
