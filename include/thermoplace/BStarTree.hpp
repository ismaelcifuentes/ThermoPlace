// ThermoPlace - B*-tree floorplan representation.
// OWNER: Ismael  (implementation goes in src/BStarTree.cpp)
//
// References: Y.-C. Chang et al., "B*-Trees: A New Representation for
// Non-Slicing Floorplans", DAC 2000; T.-C. Chen and Y.-W. Chang, "Modern Floorplanning Based on
// B*-Tree and Fast Simulated Annealing", IEEE TCAD 2006.
//
// Rules of a B*-tree (admissible, "compacted to the bottom-left"):
//   * The root block is placed at (0, 0).
//   * LEFT child of node n  -> placed immediately to the RIGHT of n:
//         x(left) = x(n) + w(n)
//   * RIGHT child of node n -> placed ABOVE n, with the same x:
//         x(right) = x(n)
//   * y of every block = highest point of the contour in [x, x + w).
//   * Blocks are placed in DFS pre-order (node, then left subtree, then right).
//
// Nodes are stored in a vector and linked by indices (-1 = no link). Using
// indices instead of raw pointers lets us copy a whole tree with `=`, which
// simulated annealing needs to keep the best solution (week 3).
#pragma once

#include <string>
#include <vector>

#include "thermoplace/Design.hpp"

namespace tp {

class BStarTree {
public:
    struct Node {
        int block = -1;   // index into Design::blocks of the block on this node
        int parent = -1;
        int left = -1;    // placed to the RIGHT of this block
        int right = -1;   // placed ABOVE this block
    };

    // Builds a valid starting tree for n blocks: a complete binary tree where
    // node i holds block i and has children 2i+1 (left) and 2i+2 (right).
    void buildInitial(int numBlocks);

    // Computes Block::x and Block::y for every block of `d` following the
    // rules above. Must use Block::w()/h() so rotation is respected.
    // Precondition: size() == d.blocks.size().
    void pack(Design& d) const;

    // --- Perturbations (needed for simulated annealing, week 3) ------------
    // Swaps the blocks stored in two nodes (tree shape does not change).
    void swapBlocks(int nodeA, int nodeB);
    // EXTRA (week 3): detaches `node` (must be a leaf for the simple version)
    // and re-inserts it as left/right child of `newParent` if that slot is
    // free. Returns false if the move is not possible.
    bool moveNode(int node, int newParent, bool asLeftChild);

    // --- Queries -------------------------------------------------------------
    int size() const { return static_cast<int>(nodes_.size()); }
    int root() const { return root_; }
    const Node& node(int i) const { return nodes_[i]; }

    // true if: root has no parent, every node is reachable from the root
    // exactly once, parent/child links agree, and every block appears once.
    bool isValid() const;

    // Human readable dump, one line per node, for debugging and the video.
    std::string toString() const;

private:
    std::vector<Node> nodes_;
    int root_ = -1;
};

}  // namespace tp
