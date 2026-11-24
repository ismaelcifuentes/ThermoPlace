"""ThermoPlace - figures 3, 3b, 4 and 6 of the final report (Jose).

usage: python3 docs/figures/scripts/make_figures.py <data dir> <output dir>

<data dir> is what docs/figures/README.md builds (all with tag v1.0):
    ami33_w0_s5_cells.csv / _blocks.csv      thermal_grid, weight 0
    ami33_w0.25_s5_cells.csv / _blocks.csv   thermal_grid, weight 0.25
    j15_grids.csv  folders   build/j15/<bench>_s<seed>/ (tools/run_j15.sh grids)
    ami33_trace.csv          SA trace of ami33, seed 1
Writes PNG (for the slides) and SVG (for the report) of every figure.
"""
import csv
import os
import sys

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt  # noqa: E402
from matplotlib.colors import LinearSegmentedColormap  # noqa: E402
from matplotlib.patches import Rectangle  # noqa: E402

# Palette (dataviz reference instance, light mode).
SURFACE = "#fcfcfb"
INK = "#0b0b0b"
INK_2 = "#52514e"
MUTED = "#898781"
GRID = "#e1e0d9"
AXIS = "#c3c2b7"
BLUE = "#2a78d6"     # categorical slot 1
ORANGE = "#eb6834"   # categorical slot 2
# Sequential ramps, light -> dark: temperature in orange, power density in blue.
TEMP_CMAP = LinearSegmentedColormap.from_list(
    "temp", ["#fdeee6", "#f8c4a8", "#f29467", "#eb6834", "#c24d1d", "#8f3412", "#5e1f08"])
POWER_CMAP = LinearSegmentedColormap.from_list(
    "power", ["#cde2fb", "#9ec5f4", "#6da7ec", "#3987e5", "#256abf", "#184f95", "#0d366b"])

plt.rcParams.update({
    "font.family": "DejaVu Sans", "font.size": 11,
    "figure.facecolor": SURFACE, "axes.facecolor": SURFACE, "savefig.facecolor": SURFACE,
    "text.color": INK, "axes.labelcolor": INK_2, "xtick.color": MUTED, "ytick.color": MUTED,
    "axes.edgecolor": AXIS, "axes.titlesize": 12, "axes.titlecolor": INK,
    "svg.fonttype": "none",
})


def read_csv(path):
    with open(path) as f:
        return list(csv.DictReader(f))


def save(fig, out, name):
    for ext in ("png", "svg"):
        fig.savefig(os.path.join(out, f"{name}.{ext}"), dpi=200, bbox_inches="tight")
    plt.close(fig)
    print("wrote", name)


def die_size(cells):
    w = max(float(c["x"]) + float(c["w"]) for c in cells)
    h = max(float(c["y"]) + float(c["h"]) for c in cells)
    return w, h


def draw_heat(ax, cells, blocks, vmin, vmax):
    """Heat map of the die (row 0 at the bottom) with the block outlines on top."""
    for c in cells:
        t = (float(c["temp_c"]) - vmin) / (vmax - vmin)
        ax.add_patch(Rectangle((float(c["x"]), float(c["y"])), float(c["w"]), float(c["h"]),
                               facecolor=TEMP_CMAP(t), edgecolor="none"))
    for b in blocks:
        ax.add_patch(Rectangle((float(b["x"]), float(b["y"])), float(b["w"]), float(b["h"]),
                               facecolor="none", edgecolor=INK, linewidth=0.5, alpha=0.55))
    hot = max(cells, key=lambda c: float(c["temp_c"]))
    ax.plot(float(hot["x"]) + float(hot["w"]) / 2, float(hot["y"]) + float(hot["h"]) / 2,
            marker="o", markersize=9, markerfacecolor="none", markeredgecolor=INK,
            markeredgewidth=2, clip_on=False, zorder=5)
    w, h = die_size(cells)
    ax.set_xlim(0, w)
    ax.set_ylim(0, h)
    ax.set_aspect("equal")
    ax.set_xticks([])
    ax.set_yticks([])


def figure3(data, out):
    cells = read_csv(os.path.join(data, "ami33_w0.25_s5_cells.csv"))
    blocks = read_csv(os.path.join(data, "ami33_w0.25_s5_blocks.csv"))
    temps = [float(c["temp_c"]) for c in cells]
    dens = [float(b["density_w_mm2"]) for b in blocks]
    w, h = die_size(cells)

    fig, (left, right) = plt.subplots(1, 2, figsize=(11, 5.2))
    # Left: the floorplan, each block coloured by its power density.
    for b, d in zip(blocks, dens):
        t = (d - 0.1) / (10 - 0.1)
        x, y, bw, bh = (float(b[k]) for k in ("x", "y", "w", "h"))
        left.add_patch(Rectangle((x, y), bw, bh, facecolor=POWER_CMAP(t), edgecolor=SURFACE,
                                 linewidth=1.5))
        left.text(x + bw / 2, y + bh / 2, b["name"], ha="center", va="center", fontsize=6.5,
                  color="#ffffff" if t > 0.45 else INK)
    left.add_patch(Rectangle((0, 0), w, h, facecolor="none", edgecolor=MUTED, linewidth=1,
                             linestyle=(0, (4, 3))))
    left.set_xlim(0, w)
    left.set_ylim(0, h)
    left.set_aspect("equal")
    left.set_xticks([])
    left.set_yticks([])
    left.set_title("Floorplan: power density per block", loc="left")
    sm = plt.cm.ScalarMappable(cmap=POWER_CMAP, norm=plt.Normalize(0.1, 10))
    cb = fig.colorbar(sm, ax=left, fraction=0.046, pad=0.03)
    cb.set_label("W/mm²")
    cb.outline.set_visible(False)

    # Right: the temperature map of the same die, with the block outlines.
    draw_heat(right, cells, blocks, min(temps), max(temps))
    right.set_title(f"Temperature (peak {max(temps):.1f} °C, circled)", loc="left")
    sm = plt.cm.ScalarMappable(cmap=TEMP_CMAP, norm=plt.Normalize(min(temps), max(temps)))
    cb = fig.colorbar(sm, ax=right, fraction=0.046, pad=0.03)
    cb.set_label("°C")
    cb.outline.set_visible(False)

    fig.suptitle("ami33 after thermal-aware SA: the peak sits on the densest block (bk1, 10 W/mm²)",
                 x=0.02, ha="left", fontsize=14, fontweight="bold")
    fig.text(0.02, 0.005, "v1.0, seed 5, thermal weight 0.25, power 0.1–10 W/mm² (6.91 W), "
             "32 × 32 grid; dashed line = fixed outline (die)", fontsize=9, color=INK_2)
    save(fig, out, "fig3_ami33_floorplan_heatmap")


def figure3b(data, out):
    maps = []
    for w in ("0", "0.25"):
        maps.append((w, read_csv(os.path.join(data, f"ami33_w{w}_s5_cells.csv")),
                     read_csv(os.path.join(data, f"ami33_w{w}_s5_blocks.csv"))))
    allt = [float(c["temp_c"]) for _, cells, _ in maps for c in cells]
    vmin, vmax = min(allt), max(allt)  # one shared scale, so the colours compare

    fig, axes = plt.subplots(1, 2, figsize=(11, 5.2))
    for ax, (w, cells, blocks) in zip(axes, maps):
        draw_heat(ax, cells, blocks, vmin, vmax)
        peak = max(float(c["temp_c"]) for c in cells)
        ax.set_title(f"Thermal weight {w}: peak {peak:.1f} °C", loc="left")
    sm = plt.cm.ScalarMappable(cmap=TEMP_CMAP, norm=plt.Normalize(vmin, vmax))
    cb = fig.colorbar(sm, ax=list(axes), fraction=0.025, pad=0.02)
    cb.set_label("°C (same scale in both)")
    cb.outline.set_visible(False)
    fig.suptitle("A thermal weight of 0.25 cools ami33's hot spot by 4 °C; HotSpot confirms "
                 "−3.7 °C over 5 seeds", x=0.02, ha="left", fontsize=14, fontweight="bold")
    fig.text(0.02, 0.005, "v1.0, seed 5, power 0.1–10 W/mm²; our model, 32 × 32 grid. "
             "HotSpot mean over seeds 1–5: 82.2 → 78.5 °C (block-level peak)",
             fontsize=9, color=INK_2)
    save(fig, out, "fig3b_ami33_thermal_weight")


def figure4(data, out):
    # Mean absolute error and solve time per grid, from the J15 runs (seeds 1-5).
    root = os.path.join(data, "j15")
    import importlib.util
    spec = importlib.util.spec_from_file_location(
        "j15_compare", os.path.join(os.path.dirname(__file__), "..", "..", "..", "tools",
                                    "j15_compare.py"))
    j15 = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(j15)
    errors = j15.compare(root, "j15_grids.csv")
    grids = sorted({k[0] for k in errors})

    def series(bench, field):
        return [j15.mean([e[field] for e in errors[k] if e["bench"] == bench])
                for k in sorted(errors)]

    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(11, 4.4))
    for bench, color in (("ami33", BLUE), ("ami49", ORANGE)):
        mae = series(bench, "mae")
        ms = series(bench, "ms")
        ax1.plot(grids, mae, color=color, linewidth=2, marker="o", markersize=8,
                 markeredgecolor=SURFACE, markeredgewidth=2, label=bench)
        ax2.plot(grids, ms, color=color, linewidth=2, marker="o", markersize=8,
                 markeredgecolor=SURFACE, markeredgewidth=2, label=bench)
        ax1.annotate(f"{bench} {mae[1]:.2f} °C", (16, mae[1]), textcoords="offset points",
                     xytext=(8, 8), fontsize=10, color=INK_2)
        ax2.annotate(f"{bench} {ms[1]:.2f} ms", (16, ms[1]), textcoords="offset points",
                     xytext=(8, -14), fontsize=10, color=INK_2)
    for ax in (ax1, ax2):
        ax.set_xscale("log", base=2)
        ax.set_xticks(grids)
        ax.set_xticklabels([f"{g}×{g}" for g in grids])
        ax.set_xlabel("grid size N × N")
        ax.grid(axis="y", color=GRID, linewidth=0.8)
        ax.spines[["top", "right"]].set_visible(False)
        ax.axvline(16, color=AXIS, linewidth=1, linestyle=(0, (4, 3)), zorder=0)
    ax1.legend(frameon=False, loc="lower right")
    ax2.legend(frameon=False, loc="upper left")
    ax1.set_ylim(0, None)
    ax1.set_ylabel("mean abs. error vs. HotSpot (°C)")
    ax1.set_title("Error per block", loc="left")
    ax2.set_yscale("log")
    ax2.set_ylabel("solve time (ms, log scale)")
    ax2.set_title("Time per thermal solve", loc="left")
    fig.suptitle("A 16 × 16 grid is enough: same 0.53 °C error as 64 × 64, 60–75× faster",
                 x=0.02, ha="left", fontsize=14, fontweight="bold")
    fig.text(0.02, -0.03, "v1.0; best SA floorplans of ami33 and ami49, seeds 1–5; "
             "power 0.5–2 W/mm²; h = 1.4·10⁵ W/(m²·K), R = 0.20 K/W; HotSpot v7.0 block mode",
             fontsize=9, color=INK_2)
    fig.tight_layout()
    save(fig, out, "fig4_error_vs_grid")


def figure6(data, out):
    rows = read_csv(os.path.join(data, "ami33_trace.csv"))
    step = [int(r["step"]) for r in rows]
    cur = [float(r["current_cost"]) for r in rows]
    best = [float(r["best_cost"]) for r in rows]
    fig, ax = plt.subplots(figsize=(9, 4.4))
    ax.plot(step, cur, color=BLUE, linewidth=1.5, label="current cost")
    ax.plot(step, best, color=ORANGE, linewidth=2, label="best cost")
    ax.annotate(f"best at start {best[0]:.2f}", (0, best[0]), xytext=(8, 0.62),
                fontsize=10, color=INK_2,
                arrowprops=dict(arrowstyle="-", color=MUTED, linewidth=0.8))
    ax.annotate(f"final {best[-1]:.2f}", (step[-1], best[-1]), textcoords="offset points",
                xytext=(-60, 12), fontsize=10, color=INK_2)
    ax.set_xlabel("temperature step")
    ax.set_ylabel("cost")
    ax.set_ylim(0, None)
    ax.grid(axis="y", color=GRID, linewidth=0.8)
    ax.spines[["top", "right"]].set_visible(False)
    ax.legend(frameon=False, loc="upper right")
    drop = 100 * (1 - best[-1] / best[0])
    fig.suptitle(f"SA on ami33 cuts the cost by {drop:.0f} % in {len(step)} temperature steps",
                 x=0.02, ha="left", fontsize=14, fontweight="bold")
    fig.text(0.02, -0.03, "v1.0, seed 1, --sa --outline-weight 5 (cooling 0.95, 10·n moves "
             "per temperature); cost = area + HPWL + outline penalty, normalized",
             fontsize=9, color=INK_2)
    fig.tight_layout()
    save(fig, out, "fig6_ami33_convergence")


if __name__ == "__main__":
    if len(sys.argv) != 3:
        sys.exit(__doc__)
    data, out = sys.argv[1], sys.argv[2]
    os.makedirs(out, exist_ok=True)
    figure3(data, out)
    figure3b(data, out)
    figure4(data, out)
    figure6(data, out)
