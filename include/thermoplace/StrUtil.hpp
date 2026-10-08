// ThermoPlace - small string helpers for the parsers.
// OWNER: shared (already implemented in the skeleton).
#pragma once

#include <sstream>
#include <string>
#include <vector>

namespace tp {

// Removes spaces, tabs and '\r' at both ends. The MCNC files use Windows line
// endings (\r\n), so ALWAYS trim each line you read.
inline std::string trim(const std::string& s) {
    const char* ws = " \t\r\n";
    std::size_t b = s.find_first_not_of(ws);
    if (b == std::string::npos) return "";
    std::size_t e = s.find_last_not_of(ws);
    return s.substr(b, e - b + 1);
}

// Splits on whitespace. "bk1   336  133" -> {"bk1", "336", "133"}
inline std::vector<std::string> splitWs(const std::string& s) {
    std::vector<std::string> out;
    std::istringstream in(s);
    std::string tok;
    while (in >> tok) out.push_back(tok);
    return out;
}

inline bool startsWith(const std::string& s, const std::string& prefix) {
    return s.size() >= prefix.size() && s.compare(0, prefix.size(), prefix) == 0;
}

inline bool endsWith(const std::string& s, const std::string& suffix) {
    return s.size() >= suffix.size() &&
           s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0;
}

// Reads the number after the ':' in lines like "NumBlocks: 33" or
// "NumHardRectilinearBlocks : 100". Returns -1 if there is no ':'.
inline long valueAfterColon(const std::string& line) {
    std::size_t p = line.find(':');
    if (p == std::string::npos) return -1;
    return std::stol(trim(line.substr(p + 1)));
}

}  // namespace tp
