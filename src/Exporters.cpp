// ThermoPlace - SVG / HotSpot / CSV writers.
// OWNER: Jose. Follow docs/MANUAL_JOSE.md. Contract: include/thermoplace/Exporters.hpp
#include "thermoplace/Exporters.hpp"

#include <algorithm>  // std::max
#include <fstream>

#include "thermoplace/Metrics.hpp"

namespace tp {

bool exportSVG(const Design& d, const std::string& path, std::string& err) {
    std::ofstream out(path);
    if (!out) {
        err = "cannot write " + path;
        return false;
    }

    Metrics m = computeMetrics(d);

    // Scale the chip so its longest side is 800 pixels, with a margin around it.
    const double pad = 40.0;
    double longest = std::max(m.chipW, m.chipH);
    double s = (longest > 0) ? 800.0 / longest : 1.0;
    double svgW = m.chipW * s + 2 * pad;
    double svgH = m.chipH * s + 2 * pad;

    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"" << svgW
        << "\" height=\"" << svgH << "\">\n";
    out << "<rect width=\"100%\" height=\"100%\" fill=\"white\"/>\n";

    // Title: design name, chip area and dead space.
    out << "<text x=\"" << pad << "\" y=\"" << pad / 2 << "\" font-size=\"14\">"
        << d.name << "  area=" << m.chipArea << "  dead space="
        << m.deadSpacePct << "%</text>\n";

    // Chip outline (dashed line).
    out << "<rect x=\"" << pad << "\" y=\"" << pad << "\" width=\"" << m.chipW * s
        << "\" height=\"" << m.chipH * s
        << "\" fill=\"none\" stroke=\"black\" stroke-dasharray=\"4\"/>\n";

    // One rectangle and one label per block.
    for (const Block& b : d.blocks) {
        double xs = pad + b.x * s;
        // SVG's y axis points DOWN, so we flip it: top edge = chipH - (y + h).
        double ys = pad + (m.chipH - (b.y + b.h())) * s;
        double ws = b.w() * s;
        double hs = b.h() * s;

        out << "<rect x=\"" << xs << "\" y=\"" << ys << "\" width=\"" << ws
            << "\" height=\"" << hs << "\" fill=\"#9ecae1\" stroke=\"#08519c\"/>\n";
        out << "<text x=\"" << xs + ws / 2 << "\" y=\"" << ys + hs / 2
            << "\" font-size=\"10\" text-anchor=\"middle\" dominant-baseline=\"middle\">"
            << b.name << "</text>\n";
    }

    out << "</svg>\n";
    return true;
}

bool exportHotSpotFLP(const Design& d, const std::string& path, double unitToMeters,
                      std::string& err) {
    std::ofstream out(path);
    if (!out) {
        err = "cannot write " + path;
        return false;
    }

    // HotSpot reads one block per line, all sizes in meters, separated by tabs.
    out.precision(9);
    out << "# ThermoPlace floorplan for HotSpot: " << d.name << "\n";
    out << "# <name>\t<width>\t<height>\t<left-x>\t<bottom-y>  (meters)\n";

    for (const Block& b : d.blocks) {
        out << b.name << "\t"
            << b.w() * unitToMeters << "\t"
            << b.h() * unitToMeters << "\t"
            << b.x * unitToMeters << "\t"
            << b.y * unitToMeters << "\n";
    }
    return true;
}

bool exportPTrace(const Design& d, const std::string& path, std::string& err) {
    // TODO(Jose, EXTRA): task J7.
    (void)d; (void)path;
    err = "exportPTrace not implemented yet (TODO Jose, extra)";
    return false;
}

bool exportCSV(const Design& d, const std::string& path, std::string& err) {
    // TODO(Jose, EXTRA): task J6.
    (void)d; (void)path;
    err = "exportCSV not implemented yet (TODO Jose, extra)";
    return false;
}

}  // namespace tp
