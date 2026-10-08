// ThermoPlace - benchmark readers.
// OWNER: Eimi  (implementation goes in src/Parser.cpp)
//
// Two benchmark families are supported:
//
//  1) MCNC  (ami33, ami49, apte, hp, xerox)   files: <name>.block + <name>.nets
//       Outline: 1326 1205
//       NumBlocks: 33
//       NumTerminals: 40
//       bk1   336  133            <- name width height
//       ...
//       VSS terminal 1410 1610    <- name "terminal" x y
//
//  2) GSRC  (n100, n200, n300)               files: <name>.hardblocks + .nets + .pl
//       NumHardRectilinearBlocks : 100
//       NumTerminals : 334
//       sb0 hardrectilinear 4 (0, 0) (0, 33) (43, 33) (43, 0)   <- width 43, height 33
//       p1 terminal                                            <- position is in .pl
//
//  Both use the same .nets format:
//       NumNets : 885
//       NetDegree : 2
//       p1          <- first token of each line is a block or terminal name
//       sb26
//
// All functions return true on success. On failure they return false and put a
// human readable message in `err` (include the file name and line number).
#pragma once

#include <string>

#include "thermoplace/Design.hpp"

namespace tp {

// --- MCNC (required for review 1) ------------------------------------------
// Reads <name>.block: fills d.outlineW/H, d.blocks (via d.addBlock) and
// d.terminals (via d.addTerminal, with x,y).
bool parseMcncBlocks(const std::string& path, Design& d, std::string& err);

// --- Nets (required for review 1, same format for MCNC and GSRC) -----------
// Must be called AFTER the blocks (and terminals) are loaded. Names that are
// neither a block nor a terminal are an error.
bool parseNets(const std::string& path, Design& d, std::string& err);

// --- GSRC (extra; needed in week 7 for the n100..n300 scalability study) ---
// Reads <name>.hardblocks: blocks (width/height from the 4 corner points) and
// terminal names (positions come later from the .pl file).
bool parseGsrcHardBlocks(const std::string& path, Design& d, std::string& err);
// Reads <name>.pl: sets x,y of the terminals already registered.
bool parseGsrcPl(const std::string& path, Design& d, std::string& err);

// --- Convenience ------------------------------------------------------------
// `path` is the blocks file: "benchmarks/mcnc/ami33.block" or
// "benchmarks/gsrc/n100.hardblocks". Detects the format from the extension,
// loads the sibling files (.nets, and .pl for GSRC) and sets d.name ("ami33").
bool loadBenchmark(const std::string& path, Design& d, std::string& err);

}  // namespace tp
