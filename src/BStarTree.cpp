// ThermoPlace - B*-tree representation and packing.
// OWNER: Ismael. Follow docs/MANUAL_ISMAEL.md. Contract: include/thermoplace/BStarTree.hpp
#include "thermoplace/BStarTree.hpp"

#include <sstream>

#include "thermoplace/Contour.hpp"

namespace tp {

void BStarTree::buildInitial(int numBlocks) {
    // TODO(Ismael): task I2 (complete binary tree: children 2i+1 and 2i+2).
    (void)numBlocks;
    nodes_.clear();
    root_ = -1;
}

void BStarTree::pack(Design& d) const {
    // TODO(Ismael): task I3 (DFS pre-order + Contour).
    (void)d;
}

void BStarTree::swapBlocks(int nodeA, int nodeB) {
    // TODO(Ismael): task I5.
    (void)nodeA; (void)nodeB;
}

bool BStarTree::moveNode(int node, int newParent, bool asLeftChild) {
    // TODO(Ismael, EXTRA week 3): task I7.
    (void)node; (void)newParent; (void)asLeftChild;
    return false;
}

bool BStarTree::isValid() const {
    // TODO(Ismael): task I4.
    return false;
}

std::string BStarTree::toString() const {
    // TODO(Ismael): task I4. One line per node: "node 0 block 0 parent -1 left 1 right 2"
    return "BStarTree::toString not implemented yet (TODO Ismael)\n";
}

}  // namespace tp
