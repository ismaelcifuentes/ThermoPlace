// ThermoPlace - benchmark readers.
// OWNER: Eimi. Follow docs/MANUAL_EIMI.md. Contract:
// include/thermoplace/Parser.hpp
#include "thermoplace/Parser.hpp"

#include <fstream>
#include <stdexcept>

#include "thermoplace/StrUtil.hpp"

namespace tp {

bool parseMcncBlocks(const std::string &path, Design &d, std::string &err) {
  // 1) Open the file
  std::ifstream in(path);
  if (!in) {
    err = "cannot open " + path;
    return false;
  }

  std::string raw;    // the line exactly as it comes from the file
  int lineNumber = 0; // to report errors like "ami33.block:12"

  // 2) Read the file line by line
  while (std::getline(in, raw)) {
    ++lineNumber;
    std::string line = trim(raw); // remove spaces and the hidden '\r'
    if (line.empty() || line[0] == '#')
      continue; // skip empty lines and comments

    std::vector<std::string> t = splitWs(line); // split the line into words
    std::string where = path + ":" + std::to_string(lineNumber) + ": ";

    try {
      if (startsWith(line, "Outline")) {
        // "Outline: 1326 1205" -> the two numbers after ':'
        std::vector<std::string> v = splitWs(line.substr(line.find(':') + 1));
        if (v.size() != 2) {
          err = where + "Outline needs two numbers";
          return false;
        }
        d.outlineW = std::stod(v[0]);
        d.outlineH = std::stod(v[1]);
      } else if (startsWith(line, "NumBlocks") ||
                 startsWith(line, "NumTerminals")) {
        // header counters: not needed, skip them
      } else if (t.size() == 4 && t[1] == "terminal") {
        // "VSS terminal 1410 1610" -> a fixed pin at (x, y)
        d.addTerminal(t[0], std::stod(t[2]), std::stod(t[3]));
      } else if (t.size() == 3) {
        // "bk1 336 133" -> a block: name, width, height
        d.addBlock(t[0], std::stod(t[1]), std::stod(t[2]));
      } else {
        err = where + "unexpected line: " + line;
        return false;
      }
    } catch (const std::exception &) {
      err = where + "bad number in: " + line;
      return false;
    }
  }
  return true;
}

bool parseNets(const std::string &path, Design &d, std::string &err) {
  // TODO(Eimi): read "NetDegree : k" + k pin names per net. Task E3.
  (void)path;
  (void)d;
  err = "parseNets not implemented yet (TODO Eimi)";
  return false;
}

bool parseGsrcHardBlocks(const std::string &path, Design &d, std::string &err) {
  // TODO(Eimi, EXTRA): task E6.
  (void)path;
  (void)d;
  err = "parseGsrcHardBlocks not implemented yet (TODO Eimi, extra)";
  return false;
}

bool parseGsrcPl(const std::string &path, Design &d, std::string &err) {
  // TODO(Eimi, EXTRA): task E6.
  (void)path;
  (void)d;
  err = "parseGsrcPl not implemented yet (TODO Eimi, extra)";
  return false;
}

bool loadBenchmark(const std::string &path, Design &d, std::string &err) {
  // TODO(Eimi): detect ".block" / ".hardblocks", load siblings, set d.name.
  // Task E4.
  (void)path;
  (void)d;
  err = "loadBenchmark not implemented yet (TODO Eimi)";
  return false;
}

} // namespace tp
