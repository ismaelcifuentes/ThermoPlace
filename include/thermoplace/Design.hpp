// ThermoPlace - shared data model.
// OWNER: shared (already implemented in the skeleton). Do NOT change field names
// without telling the whole team: every module depends on this file.
#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace tp {

// A rectangular hard block (one module of the chip).
struct Block {
    std::string name;
    double width = 0.0;    // original width  (benchmark units)
    double height = 0.0;   // original height (benchmark units)
    bool rotated = false;  // true => placed rotated 90 degrees (w and h swapped)
    double x = 0.0;        // lower-left corner, computed by BStarTree::pack()
    double y = 0.0;
    double power = 0.0;    // watts (synthetic, used from week 5 on)

    // Width/height as placed (they take rotation into account). Always use these
    // when packing, drawing or measuring.
    double w() const { return rotated ? height : width; }
    double h() const { return rotated ? width : height; }
    double area() const { return width * height; }
};

// An I/O pad with a fixed position (from the benchmark).
struct Terminal {
    std::string name;
    double x = 0.0;
    double y = 0.0;
};

// A net connects blocks and/or terminals (indices into Design vectors).
struct Net {
    std::vector<int> blocks;
    std::vector<int> terminals;
};

// A whole benchmark: blocks, terminals, nets and an optional fixed outline.
struct Design {
    std::string name;       // e.g. "ami33"
    double outlineW = 0.0;  // fixed outline given by the benchmark (0 = none)
    double outlineH = 0.0;
    std::vector<Block> blocks;
    std::vector<Terminal> terminals;
    std::vector<Net> nets;
    std::unordered_map<std::string, int> blockIndex;     // name -> index in blocks
    std::unordered_map<std::string, int> terminalIndex;  // name -> index in terminals

    // Adds a block and registers its name. Returns its index.
    int addBlock(const std::string& n, double w, double h) {
        Block b;
        b.name = n;
        b.width = w;
        b.height = h;
        blocks.push_back(b);
        int id = static_cast<int>(blocks.size()) - 1;
        blockIndex[n] = id;
        return id;
    }

    // Adds a terminal and registers its name. Returns its index.
    int addTerminal(const std::string& n, double x = 0.0, double y = 0.0) {
        Terminal t;
        t.name = n;
        t.x = x;
        t.y = y;
        terminals.push_back(t);
        int id = static_cast<int>(terminals.size()) - 1;
        terminalIndex[n] = id;
        return id;
    }

    int findBlock(const std::string& n) const {
        auto it = blockIndex.find(n);
        return it == blockIndex.end() ? -1 : it->second;
    }

    int findTerminal(const std::string& n) const {
        auto it = terminalIndex.find(n);
        return it == terminalIndex.end() ? -1 : it->second;
    }

    double totalBlockArea() const {
        double a = 0.0;
        for (const Block& b : blocks) a += b.area();
        return a;
    }
};

}  // namespace tp
