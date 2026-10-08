# Benchmarks

## MCNC (`mcnc/`)

ami33, ami49, apte, hp, xerox: the classic MCNC floorplanning benchmarks.
Files: `<name>.block` (outline, block sizes, terminal positions) and `<name>.nets`.
These copies use the format of the NTU *Physical Design for Nanometer ICs*
programming assignment and were taken from
https://github.com/mirkat1206/Sequence-Pair-Floorplanner (MIT license, `input_pa2/`).
They have Windows line endings (`\r\n`).

| Benchmark | Blocks | Terminals | Nets | Total block area |
|---|---|---|---|---|
| ami33 | 33 | 40 | 121 | 1,156,449 |
| ami49 | 49 | 22 | 396 | 35,445,424 |
| apte | 9 | 73 | 96 | 46,561,628 |
| hp | 11 | 45 | 70 | 8,830,584 |
| xerox | 10 | 2 | 182 | 19,350,296 |

## GSRC (`gsrc/`)

n100, n200, n300: hard-block GSRC Bookshelf benchmarks (used for the scalability
study). Files: `.hardblocks`, `.nets`, `.pl`. Taken from
https://github.com/romulus0914/fixed-outline_floorplanning (`testcase/`).

## Tiny (`tiny/`)

`tiny3`: three hand-made blocks used by the unit tests.

## Note on power

None of these benchmarks contains power values. For the thermal experiments we
assign synthetic power densities with a fixed seed (`assignSyntheticPower`),
and report the seed and range in the paper.
