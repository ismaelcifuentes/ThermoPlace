"""ThermoPlace - draws a program run as a terminal picture for the demo slide.

OWNER: Jose. See docs/figures/README.md.

usage: python3 docs/figures/scripts/terminal_capture.py <output.txt> <command> <out prefix>
Writes <out prefix>.png (the whole run) and <out prefix>.gif (the lines appear as
in the real run, with a pause while simulated annealing works).
"""
import sys

from PIL import Image, ImageDraw, ImageFont

BG = "#1a1a19"         # dark surface
INK = "#c3c2b7"        # normal output
BRIGHT = "#f0efec"     # command
HIGHLIGHT = "#eda100"  # the lines the audience should read
PROMPT = "#3987e5"
FONT = "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf"
FONT_BOLD = "/usr/share/fonts/truetype/dejavu/DejaVuSansMono-Bold.ttf"
SIZE, PAD, LINE = 18, 28, 27

# Lines that carry the result: start, end and temperature.
KEY = ("initial ", "best ", "Thermal initial", "Thermal best", "Simulated annealing",
       "Fits outline")


def frame(lines, command, width, height, cursor=False):
    img = Image.new("RGB", (width, height), BG)
    d = ImageDraw.Draw(img)
    regular = ImageFont.truetype(FONT, SIZE)
    bold = ImageFont.truetype(FONT_BOLD, SIZE)
    y = PAD
    d.text((PAD, y), "$ ", font=bold, fill=PROMPT)
    d.text((PAD + 2 * SIZE * 0.6, y), command, font=bold, fill=BRIGHT)
    y += int(LINE * 1.5)
    for line in lines:
        key = line.startswith(KEY)
        d.text((PAD, y), line, font=bold if key else regular, fill=HIGHLIGHT if key else INK)
        y += LINE
    if cursor:
        d.rectangle((PAD, y + 4, PAD + 11, y + LINE - 4), fill=INK)
    return img


def main(output_path, command, prefix):
    with open(output_path) as f:
        lines = [l.rstrip("\n") for l in f if l.strip()]
    font = ImageFont.truetype(FONT, SIZE)
    longest = max([command] + lines, key=len)
    width = int(font.getlength("$ " + longest)) + 2 * PAD + 20
    height = PAD * 2 + int(LINE * 1.5) + LINE * (len(lines) + 1)

    frame(lines, command, width, height).save(prefix + ".png")

    # GIF: lines appear one by one; a long pause after the "Thermal cost" line,
    # where the real program runs simulated annealing for a few seconds.
    frames, durations = [frame([], command, width, height, cursor=True)], [1200]
    for i in range(1, len(lines) + 1):
        frames.append(frame(lines[:i], command, width, height, cursor=i < len(lines)))
        durations.append(1500 if lines[i - 1].startswith("Thermal cost") else 220)
    durations[-1] = 5000  # hold the final result
    frames = [f.convert("P", palette=Image.ADAPTIVE, colors=32) for f in frames]
    frames[0].save(prefix + ".gif", save_all=True, append_images=frames[1:],
                   duration=durations, loop=0, optimize=True)
    print("wrote", prefix + ".png", "and", prefix + ".gif")


if __name__ == "__main__":
    if len(sys.argv) != 4:
        sys.exit(__doc__)
    main(*sys.argv[1:])
