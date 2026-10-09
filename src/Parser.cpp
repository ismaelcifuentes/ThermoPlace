// ThermoPlace - benchmark readers.
// OWNER: Eimi. Contract: include/thermoplace/Parser.hpp
#include "thermoplace/Parser.hpp"

#include <algorithm>
#include <fstream>
#include <stdexcept>

#include "thermoplace/StrUtil.hpp"

namespace tp {

namespace {

// Every parser reports problems as "file:line: message" so a broken benchmark
// points straight at the line to fix.
bool failAt(std::string& err, const std::string& path, int lineNo, const std::string& msg) {
    err = path + ":" + std::to_string(lineNo) + ": " + msg;
    return false;
}

}  // namespace

bool parseMcncBlocks(const std::string& path, Design& d, std::string& err) {
    std::ifstream in(path);
    if (!in) {
        err = "cannot open " + path;
        return false;
    }

    std::string raw;
    int lineNo = 0;
    while (std::getline(in, raw)) {
        ++lineNo;
        // MCNC files have \r\n endings: without trim, "bk1" would be stored as "bk1\r"
        // and the .nets lookup would never find it.
        std::string line = trim(raw);
        if (line.empty() || line[0] == '#') continue;

        std::vector<std::string> t = splitWs(line);
        try {
            if (startsWith(line, "Outline")) {
                std::vector<std::string> v = splitWs(line.substr(line.find(':') + 1));
                if (v.size() != 2) return failAt(err, path, lineNo, "Outline needs width and height");
                d.outlineW = std::stod(v[0]);
                d.outlineH = std::stod(v[1]);
            } else if (startsWith(line, "NumBlocks") || startsWith(line, "NumTerminals")) {
                // Header counts are not trusted: we keep what we actually read.
            } else if (t.size() == 4 && t[1] == "terminal") {
                d.addTerminal(t[0], std::stod(t[2]), std::stod(t[3]));
            } else if (t.size() == 3) {
                d.addBlock(t[0], std::stod(t[1]), std::stod(t[2]));
            } else {
                return failAt(err, path, lineNo, "unexpected line: " + line);
            }
        } catch (const std::logic_error&) {
            // stod throws invalid_argument ("abc") or out_of_range ("1e999"), both logic_error.
            return failAt(err, path, lineNo, "not a valid number in: " + line);
        }
    }
    return true;
}

bool parseNets(const std::string& path, Design& d, std::string& err) {
    std::ifstream in(path);
    if (!in) {
        err = "cannot open " + path;
        return false;
    }

    std::string raw;
    int lineNo = 0;
    while (std::getline(in, raw)) {
        ++lineNo;
        std::string line = trim(raw);
        // Only "NetDegree: k" opens a net; NumNets/NumPins are header counts we don't need.
        if (!startsWith(line, "NetDegree")) continue;

        long degree = 0;
        try {
            degree = valueAfterColon(line);
        } catch (const std::logic_error&) {
            return failAt(err, path, lineNo, "bad NetDegree: " + line);
        }
        if (degree < 0) return failAt(err, path, lineNo, "NetDegree without ':' : " + line);

        Net net;
        long found = 0;
        while (found < degree) {
            if (!std::getline(in, raw)) {
                return failAt(err, path, lineNo, "file ended in the middle of a net");
            }
            ++lineNo;

            std::vector<std::string> t = splitWs(trim(raw));
            if (t.empty()) continue;  // a blank line is not a pin, so it must not count

            // Bookshelf pin lines may carry extra columns after the name; the name comes first.
            int b = d.findBlock(t[0]);
            int term = d.findTerminal(t[0]);
            if (b >= 0) {
                net.blocks.push_back(b);
            } else if (term >= 0) {
                net.terminals.push_back(term);
            } else {
                return failAt(err, path, lineNo,
                              "unknown pin " + t[0] + " (blocks must be loaded before nets)");
            }
            ++found;
        }
        d.nets.push_back(net);
    }
    return true;
}

bool parseGsrcHardBlocks(const std::string& path, Design& d, std::string& err) {
    std::ifstream in(path);
    if (!in) {
        err = "cannot open " + path;
        return false;
    }

    std::string raw;
    int lineNo = 0;
    while (std::getline(in, raw)) {
        ++lineNo;
        std::string line = trim(raw);
        if (line.empty() || line[0] == '#' || startsWith(line, "UCSC") || startsWith(line, "Num")) {
            continue;
        }

        std::vector<std::string> head = splitWs(line);
        if (head.size() >= 2 && head[1] == "terminal") {
            // Terminal positions live in the .pl file; parseGsrcPl fills them in later.
            d.addTerminal(head[0]);
            continue;
        }
        if (head.size() < 3 || head[1] != "hardrectilinear") {
            return failAt(err, path, lineNo, "unexpected line: " + line);
        }

        // "sb0 hardrectilinear 4 (0, 0) (0, 33) (43, 33) (43, 0)": drop the punctuation
        // and read the vertices as plain numbers.
        std::string flat = line;
        std::replace_if(flat.begin(), flat.end(),
                        [](char c) { return c == '(' || c == ')' || c == ','; }, ' ');
        std::vector<std::string> v = splitWs(flat);

        try {
            long vertices = std::stol(v[2]);
            // The B*-tree packs rectangles only; an L-shaped block would need a different model.
            if (vertices != 4) {
                return failAt(err, path, lineNo, "only rectangular blocks are supported: " + line);
            }
            if (v.size() != 3 + 2 * static_cast<std::size_t>(vertices)) {
                return failAt(err, path, lineNo, "expected 4 (x, y) points: " + line);
            }

            double minX = std::stod(v[3]), maxX = minX;
            double minY = std::stod(v[4]), maxY = minY;
            for (std::size_t i = 5; i + 1 < v.size(); i += 2) {
                double x = std::stod(v[i]);
                double y = std::stod(v[i + 1]);
                minX = std::min(minX, x);
                maxX = std::max(maxX, x);
                minY = std::min(minY, y);
                maxY = std::max(maxY, y);
            }
            d.addBlock(v[0], maxX - minX, maxY - minY);
        } catch (const std::logic_error&) {
            return failAt(err, path, lineNo, "not a valid number in: " + line);
        }
    }
    return true;
}

bool parseGsrcPl(const std::string& path, Design& d, std::string& err) {
    std::ifstream in(path);
    if (!in) {
        err = "cannot open " + path;
        return false;
    }

    std::string raw;
    int lineNo = 0;
    while (std::getline(in, raw)) {
        ++lineNo;
        std::string line = trim(raw);
        if (line.empty() || line[0] == '#' || startsWith(line, "UCLA")) continue;

        std::vector<std::string> t = splitWs(line);
        if (t.size() < 3) return failAt(err, path, lineNo, "expected 'name x y': " + line);

        int term = d.findTerminal(t[0]);
        if (term < 0) {
            // A .pl may also list block positions; we ignore them because placing
            // the blocks is exactly what the floorplanner computes.
            if (d.findBlock(t[0]) >= 0) continue;
            return failAt(err, path, lineNo, "unknown name " + t[0]);
        }
        try {
            d.terminals[term].x = std::stod(t[1]);
            d.terminals[term].y = std::stod(t[2]);
        } catch (const std::logic_error&) {
            return failAt(err, path, lineNo, "not a valid number in: " + line);
        }
    }
    return true;
}

bool loadBenchmark(const std::string& path, Design& d, std::string& err) {
    d = Design();  // reusing a Design must not mix blocks from two benchmarks

    std::size_t slash = path.find_last_of("/\\");
    std::string dir = (slash == std::string::npos) ? "" : path.substr(0, slash + 1);
    std::string file = (slash == std::string::npos) ? path : path.substr(slash + 1);

    // Sibling files share the base name (ami33.block + ami33.nets). The order matters:
    // nets refer to blocks and terminals by name, so those must exist first.
    if (endsWith(file, ".block")) {
        d.name = file.substr(0, file.size() - std::string(".block").size());
        std::string base = dir + d.name;
        return parseMcncBlocks(path, d, err) && parseNets(base + ".nets", d, err);
    }
    if (endsWith(file, ".hardblocks")) {
        d.name = file.substr(0, file.size() - std::string(".hardblocks").size());
        std::string base = dir + d.name;
        return parseGsrcHardBlocks(path, d, err) && parseGsrcPl(base + ".pl", d, err) &&
               parseNets(base + ".nets", d, err);
    }

    err = "unknown benchmark type (expected .block or .hardblocks): " + path;
    return false;
}

}  // namespace tp
