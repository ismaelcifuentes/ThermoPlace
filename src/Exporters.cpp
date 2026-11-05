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

bool exportThermalSVG(const ThermalMap& map, const std::string& path,
                      const std::string& title, std::string& err) {
    // TODO(Jose, week 4): task J12.
    (void)map; (void)path; (void)title;
    err = "exportThermalSVG not implemented yet (TODO Jose)";
    return false;
}

}  // namespace tp
