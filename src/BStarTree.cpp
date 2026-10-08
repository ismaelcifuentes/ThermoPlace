// ThermoPlace - B*-tree representation and packing.
// OWNER: Ismael. Follow docs/MANUAL_ISMAEL.md. Contract: include/thermoplace/BStarTree.hpp
#include "thermoplace/BStarTree.hpp"

#include <sstream>
#include <utility>

#include "thermoplace/Contour.hpp"

namespace tp {

// I2: complete binary tree. Node i holds block i; its children are 2i+1 (left)
// and 2i+2 (right) when those indices exist. The parent of i is (i-1)/2.
void BStarTree::buildInitial(int numBlocks) {
    nodes_.assign(numBlocks > 0 ? numBlocks : 0, Node{});
    root_ = (numBlocks > 0) ? 0 : -1;
    for (int i = 0; i < numBlocks; ++i) {
        nodes_[i].block = i;
        if (i > 0) nodes_[i].parent = (i - 1) / 2;
        if (2 * i + 1 < numBlocks) nodes_[i].left = 2 * i + 1;
        if (2 * i + 2 < numBlocks) nodes_[i].right = 2 * i + 2;
    }
}

// I3: place every block. We visit nodes in DFS pre-order (node, whole left
// subtree, then right subtree) using an explicit stack instead of recursion.
//   x: root -> 0; left child -> right of parent; right child -> same x as parent.
//   y: highest point of the contour under [x, x + w), then raise the contour.
void BStarTree::pack(Design& d) const {
    if (root_ == -1) return;

    Contour contour;
    contour.clear();

    std::vector<int> stack;
    stack.push_back(root_);

    while (!stack.empty()) {
        int n = stack.back();
        stack.pop_back();

        Block& b = d.blocks[nodes_[n].block];
        int p = nodes_[n].parent;

        double x = 0.0;
        if (p != -1) {
            const Block& pb = d.blocks[nodes_[p].block];
            if (nodes_[p].left == n) {
                x = pb.x + pb.w();  // left child: immediately to the right of the parent
            } else {
                x = pb.x;           // right child: on top of the parent, same x
            }
        }

        b.x = x;
        b.y = contour.maxHeight(x, x + b.w());  // the block "falls" until it touches
        contour.insert(x, x + b.w(), b.y + b.h());

        // Push right first so the left child is popped (placed) first.
        if (nodes_[n].right != -1) stack.push_back(nodes_[n].right);
        if (nodes_[n].left != -1) stack.push_back(nodes_[n].left);
    }
}

// I5: swap the blocks of two nodes. The shape of the tree does not change.
void BStarTree::swapBlocks(int nodeA, int nodeB) {
    std::swap(nodes_[nodeA].block, nodes_[nodeB].block);
}

bool BStarTree::moveNode(int node, int newParent, bool asLeftChild) {
    // TODO(Ismael, EXTRA week 3): task I7.
    (void)node; (void)newParent; (void)asLeftChild;
    return false;
}

// I4: checks that the tree is a correct B*-tree:
// root has no parent, every node is reached exactly once from the root,
// parent/child links agree, and every block 0..n-1 appears exactly once.
bool BStarTree::isValid() const {
    const int n = size();
    if (n == 0) return root_ == -1;
    if (root_ < 0 || root_ >= n) return false;
    if (nodes_[root_].parent != -1) return false;

    std::vector<int> seenNode(n, 0);
    std::vector<int> seenBlock(n, 0);
    std::vector<int> stack;
    stack.push_back(root_);

    while (!stack.empty()) {
        int v = stack.back();
        stack.pop_back();

        if (seenNode[v]) return false;  // reached twice -> cycle or shared child
        seenNode[v] = 1;

        int blk = nodes_[v].block;
        if (blk < 0 || blk >= n || seenBlock[blk]) return false;
        seenBlock[blk] = 1;

        int children[2] = {nodes_[v].left, nodes_[v].right};
        for (int c : children) {
            if (c == -1) continue;
            if (c < 0 || c >= n) return false;
            if (nodes_[c].parent != v) return false;  // links must agree
            stack.push_back(c);
        }
    }

    for (int i = 0; i < n; ++i) {
        if (!seenNode[i]) return false;  // node not reachable from the root
    }
    return true;
}

// I4: one line per node, e.g. "node 0 block 0 parent -1 left 1 right 2".
std::string BStarTree::toString() const {
    std::ostringstream out;
    for (int i = 0; i < size(); ++i) {
        const Node& nd = nodes_[i];
        out << "node " << i << " block " << nd.block << " parent " << nd.parent
            << " left " << nd.left << " right " << nd.right << "\n";
    }
    return out.str();
}

}  // namespace tp
