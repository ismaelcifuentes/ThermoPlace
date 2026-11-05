// ThermoPlace - SVG / HotSpot / CSV writers.
// OWNER: Jose. Follow docs/MANUAL_JOSE.md. Contract: include/thermoplace/Exporters.hpp
#include "thermoplace/Exporters.hpp"

#include <algorithm>  // std::min, std::max
#include <fstream>
#include <iomanip>    // std::fixed, std::setprecision

#include "thermoplace/Metrics.hpp"

namespace tp {

bool exportSVG(const Design& d, const std::string& path, std::string& err) {
    std::ofstream out(path);
    if (!out) {
        err = "cannot write " + path;
        return false;
    }

    Metrics m = computeMetrics(d);

    // The drawing must hold both the chip and the fixed outline (if there is one).
    double frameW = std::max(m.chipW, d.outlineW);
    double frameH = std::max(m.chipH, d.outlineH);

    // Scale the frame so its longest side is 800 pixels, with a margin around it.
    const double pad = 40.0;
    double longest = std::max(frameW, frameH);
    double s = (longest > 0) ? 800.0 / longest : 1.0;
    double svgW = frameW * s + 2 * pad;
    double svgH = frameH * s + 2 * pad;

    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"" << svgW
        << "\" height=\"" << svgH << "\">\n";
    out << "<rect width=\"100%\" height=\"100%\" fill=\"white\"/>\n";

    // Title: design name, chip area, dead space and wirelength.
    // std::fixed avoids scientific notation like 1.35808e+06 in the figure.
    out << std::fixed << std::setprecision(0);
    out << "<text x=\"" << pad << "\" y=\"" << pad / 2 << "\" font-size=\"14\">"
        << d.name << "  area=" << m.chipArea << "  HPWL=" << m.hpwl
        << std::setprecision(2) << "  dead space=" << m.deadSpacePct << "%</text>\n";

    // Chip bounding box (black dashed line). In SVG the bottom of the frame
    // is at pad + frameH * s, so a box of height H starts at that minus H.
    out << "<rect x=\"" << pad << "\" y=\"" << pad + (frameH - m.chipH) * s
        << "\" width=\"" << m.chipW * s << "\" height=\"" << m.chipH * s
        << "\" fill=\"none\" stroke=\"black\" stroke-dasharray=\"4\"/>\n";

    // Fixed outline (red dashed line), only when the benchmark gives one.
    if (d.outlineW > 0 && d.outlineH > 0) {
        out << "<rect x=\"" << pad << "\" y=\"" << pad + (frameH - d.outlineH) * s
            << "\" width=\"" << d.outlineW * s << "\" height=\"" << d.outlineH * s
            << "\" fill=\"none\" stroke=\"red\" stroke-dasharray=\"8 4\"/>\n";
    }

    // One rectangle and one label per block.
    for (const Block& b : d.blocks) {
        double xs = pad + b.x * s;
        // SVG's y axis points DOWN, so we flip it: top edge = frameH - (y + h).
        double ys = pad + (frameH - (b.y + b.h())) * s;
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
    std::ofstream out(path);
    if (!out) {
        err = "cannot write " + path;
        return false;
    }

    // Line 1: block names, separated by tabs (HotSpot matches them with the .flp).
    for (std::size_t i = 0; i < d.blocks.size(); ++i) {
        if (i > 0) out << "\t";
        out << d.blocks[i].name;
    }
    out << "\n";

    // Line 2: the power of each block in watts, in the same order.
    for (std::size_t i = 0; i < d.blocks.size(); ++i) {
        if (i > 0) out << "\t";
        out << d.blocks[i].power;
    }
    out << "\n";
    return true;
}

bool exportCSV(const Design& d, const std::string& path, std::string& err) {
    std::ofstream out(path);
    if (!out) {
        err = "cannot write " + path;
        return false;
    }

    // One row per block. w and h are the PLACED size (already rotated).
    out << "name,x,y,w,h,rotated,power\n";
    for (const Block& b : d.blocks) {
        out << b.name << "," << b.x << "," << b.y << "," << b.w() << "," << b.h()
            << "," << (b.rotated ? 1 : 0) << "," << b.power << "\n";
    }
    return true;
}

}  // namespace tp

namespace tp {

bool exportConvergenceSVG(const std::vector<SATracePoint>& trace, const std::string& path,
                          const std::string& title, std::string& err) {
    if (trace.empty()) {
        err = "empty trace, nothing to plot";
        return false;
    }
    std::ofstream out(path);
    if (!out) {
        err = "cannot write " + path;
        return false;
    }

    // Range of the data: x = temperature step, y = cost (current and best).
    int minStep = trace.front().step;
    int maxStep = trace.front().step;
    double minCost = trace.front().currentCost;
    double maxCost = trace.front().currentCost;
    for (const SATracePoint& p : trace) {
        minStep = std::min(minStep, p.step);
        maxStep = std::max(maxStep, p.step);
        minCost = std::min(minCost, std::min(p.currentCost, p.bestCost));
        maxCost = std::max(maxCost, std::max(p.currentCost, p.bestCost));
    }
    // Avoid dividing by zero when all values are equal (or there is one point).
    double stepRange = (maxStep > minStep) ? double(maxStep - minStep) : 1.0;
    double costRange = (maxCost > minCost) ? (maxCost - minCost) : 1.0;

    // Plot area inside the picture (left margin is wider for the y labels).
    const double width = 800.0, height = 500.0;
    const double left = 80.0, right = 20.0, top = 50.0, bottom = 50.0;
    const double plotW = width - left - right;
    const double plotH = height - top - bottom;

    // Data -> pixels. SVG's y axis points DOWN, so a high cost goes near the top.
    auto px = [&](int step) { return left + (step - minStep) / stepRange * plotW; };
    auto py = [&](double c) { return top + (maxCost - c) / costRange * plotH; };

    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"" << width
        << "\" height=\"" << height << "\">\n";
    out << "<rect width=\"100%\" height=\"100%\" fill=\"white\"/>\n";

    // Title.
    out << "<text x=\"" << width / 2 << "\" y=\"" << top / 2
        << "\" font-size=\"16\" text-anchor=\"middle\">" << title << "</text>\n";

    // Axes: y on the left, x at the bottom.
    out << "<line x1=\"" << left << "\" y1=\"" << top << "\" x2=\"" << left
        << "\" y2=\"" << top + plotH << "\" stroke=\"black\"/>\n";
    out << "<line x1=\"" << left << "\" y1=\"" << top + plotH << "\" x2=\""
        << left + plotW << "\" y2=\"" << top + plotH << "\" stroke=\"black\"/>\n";

    // Min and max cost written on the y axis, first and last step on the x axis.
    // Three decimals are enough for a normalized cost (e.g. 0.331 instead of 0.330514).
    out << std::fixed << std::setprecision(3);
    out << "<text x=\"" << left - 5 << "\" y=\"" << top
        << "\" font-size=\"11\" text-anchor=\"end\" dominant-baseline=\"middle\">"
        << maxCost << "</text>\n";
    out << "<text x=\"" << left - 5 << "\" y=\"" << top + plotH
        << "\" font-size=\"11\" text-anchor=\"end\" dominant-baseline=\"middle\">"
        << minCost << "</text>\n";
    out << "<text x=\"" << left << "\" y=\"" << top + plotH + 15
        << "\" font-size=\"11\" text-anchor=\"middle\">" << minStep << "</text>\n";
    out << "<text x=\"" << left + plotW << "\" y=\"" << top + plotH + 15
        << "\" font-size=\"11\" text-anchor=\"middle\">" << maxStep << "</text>\n";

    // Axis names.
    out << "<text x=\"" << left + plotW / 2 << "\" y=\"" << height - 10
        << "\" font-size=\"12\" text-anchor=\"middle\">temperature step</text>\n";
    out << "<text x=\"15\" y=\"" << top + plotH / 2
        << "\" font-size=\"12\" text-anchor=\"middle\" transform=\"rotate(-90 15 "
        << top + plotH / 2 << ")\">cost</text>\n";

    // Line 1: current cost (blue).
    out << "<polyline fill=\"none\" stroke=\"#3182bd\" stroke-width=\"1.5\" points=\"";
    for (const SATracePoint& p : trace) out << px(p.step) << "," << py(p.currentCost) << " ";
    out << "\"/>\n";

    // Line 2: best cost so far (red).
    out << "<polyline fill=\"none\" stroke=\"#de2d26\" stroke-width=\"2\" points=\"";
    for (const SATracePoint& p : trace) out << px(p.step) << "," << py(p.bestCost) << " ";
    out << "\"/>\n";

    // Legend in the top-right corner of the plot.
    double lx = left + plotW - 150, ly = top + 15;
    out << "<line x1=\"" << lx << "\" y1=\"" << ly << "\" x2=\"" << lx + 25 << "\" y2=\""
        << ly << "\" stroke=\"#3182bd\" stroke-width=\"1.5\"/>\n";
    out << "<text x=\"" << lx + 30 << "\" y=\"" << ly
        << "\" font-size=\"12\" dominant-baseline=\"middle\">current cost</text>\n";
    out << "<line x1=\"" << lx << "\" y1=\"" << ly + 20 << "\" x2=\"" << lx + 25 << "\" y2=\""
        << ly + 20 << "\" stroke=\"#de2d26\" stroke-width=\"2\"/>\n";
    out << "<text x=\"" << lx + 30 << "\" y=\"" << ly + 20
        << "\" font-size=\"12\" dominant-baseline=\"middle\">best cost</text>\n";

    out << "</svg>\n";
    return true;
}

}  // namespace tp

namespace tp {

// Colour for a value t between 0 (coolest) and 1 (hottest), as "rgb(r,g,b)".
// It goes blue -> cyan -> green -> yellow -> red, in 4 equal steps.
static std::string heatColor(double t) {
    t = std::max(0.0, std::min(1.0, t));
    double r = 0, g = 0, b = 0;
    if (t < 0.25) {         // blue -> cyan: green goes up
        b = 1; g = t / 0.25;
    } else if (t < 0.5) {   // cyan -> green: blue goes down
        g = 1; b = 1 - (t - 0.25) / 0.25;
    } else if (t < 0.75) {  // green -> yellow: red goes up
        g = 1; r = (t - 0.5) / 0.25;
    } else {                // yellow -> red: green goes down
        r = 1; g = 1 - (t - 0.75) / 0.25;
    }
    return "rgb(" + std::to_string(static_cast<int>(r * 255)) + "," +
           std::to_string(static_cast<int>(g * 255)) + "," +
           std::to_string(static_cast<int>(b * 255)) + ")";
}

bool exportThermalSVG(const ThermalMap& map, const std::string& path,
                      const std::string& title, std::string& err) {
    if (map.n == 0 || map.temp.empty()) {
        err = "empty thermal map, nothing to draw";
        return false;
    }
    std::ofstream out(path);
    if (!out) {
        err = "cannot write " + path;
        return false;
    }

    // 1. Coolest and hottest cell: they are the two ends of the colour scale.
    double minT = map.temp[0];
    double maxT = map.temp[0];
    for (double t : map.temp) {
        minT = std::min(minT, t);
        maxT = std::max(maxT, t);
    }
    // A uniform map (all cells equal) would divide by zero below.
    double range = (maxT > minT) ? (maxT - minT) : 1.0;

    // 2. Size of the picture: the longest side of the chip is 600 pixels.
    //    Cells keep the chip's proportions (they are not always square).
    double chipW = map.n * map.cellW;
    double chipH = map.n * map.cellH;
    double longest = std::max(chipW, chipH);
    double s = (longest > 0) ? 600.0 / longest : 1.0;
    double cw = (longest > 0) ? map.cellW * s : 600.0 / map.n;
    double ch = (longest > 0) ? map.cellH * s : 600.0 / map.n;
    const double left = 40.0, top = 50.0;
    double mapW = map.n * cw;
    double mapH = map.n * ch;
    double svgW = left + mapW + 120.0;  // room on the right for the colour scale
    double svgH = top + mapH + 40.0;

    out << std::fixed << std::setprecision(2);
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"" << svgW
        << "\" height=\"" << svgH << "\">\n";
    out << "<rect width=\"100%\" height=\"100%\" fill=\"white\"/>\n";
    out << "<text x=\"" << left << "\" y=\"30\" font-size=\"16\">" << title
        << "  Tmax=" << map.maxTemp << " C  Tavg=" << map.avgTemp << " C</text>\n";

    // 3. One rectangle per cell. Row 0 is the BOTTOM of the chip, but SVG's
    //    y axis points down, so row r is drawn (n - 1 - r) cells from the top.
    for (int row = 0; row < map.n; ++row) {
        for (int col = 0; col < map.n; ++col) {
            double x = left + col * cw;
            double y = top + (map.n - 1 - row) * ch;
            double t = (map.tempAt(row, col) - minT) / range;
            // crispEdges avoids thin white seams between neighbouring cells.
            out << "<rect class=\"cell\" shape-rendering=\"crispEdges\" x=\"" << x
                << "\" y=\"" << y << "\" width=\"" << cw << "\" height=\"" << ch
                << "\" fill=\"" << heatColor(t) << "\"/>\n";
        }
    }
    // Thin border around the chip.
    out << "<rect x=\"" << left << "\" y=\"" << top << "\" width=\"" << mapW << "\" height=\""
        << mapH << "\" fill=\"none\" stroke=\"black\"/>\n";

    // 4. Colour scale: a vertical gradient, red (hot) at the top, blue at the bottom.
    double barX = left + mapW + 20.0;
    out << "<defs><linearGradient id=\"heat\" x1=\"0\" y1=\"1\" x2=\"0\" y2=\"0\">\n";
    for (int i = 0; i <= 4; ++i) {
        out << "<stop offset=\"" << i * 25 << "%\" stop-color=\"" << heatColor(i / 4.0)
            << "\"/>\n";
    }
    out << "</linearGradient></defs>\n";
    out << "<rect x=\"" << barX << "\" y=\"" << top << "\" width=\"20\" height=\"" << mapH
        << "\" fill=\"url(#heat)\" stroke=\"black\"/>\n";
    out << "<text x=\"" << barX + 26 << "\" y=\"" << top + 10 << "\" font-size=\"12\">"
        << maxT << " C</text>\n";
    out << "<text x=\"" << barX + 26 << "\" y=\"" << top + mapH << "\" font-size=\"12\">"
        << minT << " C</text>\n";

    out << "</svg>\n";
    return true;
}

}  // namespace tp
