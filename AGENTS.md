# Rules for AI coding agents (Antigravity, Copilot, Claude, etc.)

ThermoPlace is a student research project: thermal-aware floorplanning of chips
with a B*-tree and simulated annealing, in C++17. Three people work in
parallel on separate files. Follow these rules strictly.

## 1. Ask who you are working with

The human is Ismael, Eimi or Jose. If they did not say, ask. Then read their manual:
`docs/MANUAL_ISMAEL.md`, `docs/MANUAL_EIMI.md` or `docs/MANUAL_JOSE.md`.

## 2. Only edit the files of that person

| Person | May edit |
|---|---|
| Ismael | `src/Contour.cpp`, `src/BStarTree.cpp` |
| Eimi   | `src/Parser.cpp`, `app/main.cpp`, `README.md`, `docs/REVISION1.md` |
| Jose   | `src/Metrics.cpp`, `src/Validator.cpp`, `src/Exporters.cpp`, `src/Power.cpp` |

Never edit, without the human explicitly saying the whole team agreed:
- `include/thermoplace/*.hpp` (these headers are the contract between the three people),
- anything in `tests/` (the tests define "done"; never weaken a test to make it pass),
- `CMakeLists.txt`, `build.bat`, `build.sh`, `benchmarks/`.

If a task seems to need a change in those files, stop and tell the human why.

## 3. Teach, then code

The students must explain this code in a recorded video, in English. So:
- explain the idea of each step in plain words before writing it,
- keep the code simple and readable (no clever tricks, no templates beyond the STL),
- add short comments in English,
- after writing, summarize what the function does in 2–3 sentences they can reuse.

## 4. Technical constraints

- C++17, standard library only. No external dependencies.
- Do not use `std::filesystem` (it breaks with the old MinGW bundled with Code::Blocks).
- Do not use `%zu` in printf (same reason). Prefer `std::cout`.
- Use `Block::w()` / `Block::h()` (placed size, rotation-aware), not `width`/`height`,
  except when the original size is explicitly needed.
- Benchmark files from MCNC have Windows line endings: always `trim()` lines.
- Keep the TODO task ids from the manuals (I1, E2, J4, ...) in commit messages.

## 5. Build and test before saying "done"

Windows (MinGW g++ in PATH), from the repository root, in Command Prompt:

    build.bat
    build\thermoplace_tests.exe ismael     (or eimi / jose)

In PowerShell prefix with `.\`: `.\build.bat`, `.\build\thermoplace_tests.exe ismael`.

Linux/macOS/Git Bash: `./build.sh` and `./build/thermoplace_tests ismael`.
CMake also works: `cmake -S . -B build && cmake --build build`.

A task is done only when that person's test group passes. Run the program on
the real benchmark when the integration is ready:

    build\thermoplace.exe benchmarks\mcnc\ami33.block --out results --iters 20000

## 6. Git

Each person works on their own branch (`ismael/bstar`, `eimi/parser`,
`jose/outputs`) and opens a Pull Request to `main`. The repository belongs to
Ismael (team lead); Eimi reviews and merges. Never force-push and never commit to
`main` directly (except Ismael, for the initial skeleton).
