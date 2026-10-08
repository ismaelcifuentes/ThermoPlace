# Revisión 1: checklist de entrega

Lo que pide el curso:

- [ ] Equipo de hasta 3 personas, proyecto en C++ empezado
- [ ] Código fuente subido a un repositorio de GitHub
- [ ] PDF con: título, integrantes, descripción, componentes implementados y link del repo
- [ ] En Moodle, **cada integrante** sube: (1) el PDF y (2) **su propio** video de 5 min **en inglés** explicando el objetivo y la implementación actual

## Antes de grabar

- [ ] `main` tiene los PRs de los tres integrados
- [ ] `build\thermoplace_tests.exe core` → todo en PASS (copien el resumen para el PDF)
- [ ] `build\thermoplace.exe benchmarks\mcnc\ami33.block --out results --iters 20000` funciona
- [ ] Capturas de `results\ami33_initial.svg` y `results\ami33_best.svg`
- [ ] Tabla del README completada con los números reales de ami33

---

## Plantilla del PDF (en inglés)

**Title:** ThermoPlace: Thermal-Aware Floorplanning of Chips with B\*-Trees

**Team members:** Ismael Xavier Cifuentes Carvajal, Eimi Stefany Sevilla Flores, Jose Fernando Reinoso Hernandez

**GitHub repository:** https://github.com/ismaelcifuentes/ThermoPlace

**Project description.**
Floorplanning decides where each block of a chip is placed. Classic floorplanners minimize chip area and wirelength, but heat has become one of the main limits of modern chips, especially with chiplets. ThermoPlace is a C++17 floorplanner that represents a floorplan with a B\*-tree, optimizes it with simulated annealing and, in later stages, adds a fast grid thermal model to the cost function, validated against the HotSpot thermal simulator. We evaluate it on the public MCNC (ami33, ami49, apte, hp, xerox) and GSRC (n100–n300) benchmarks, which lets us compare our results with published work. Our research question is how simple the thermal model can be while still matching HotSpot, and how much area each degree of peak temperature costs.

**Currently implemented components.**

1. *Benchmark reader*: loads MCNC `.block`/`.nets` files (blocks, terminals, nets, fixed outline).
2. *B\*-tree*: node structure linked by indices, initial complete-binary-tree construction, validity check and block-swap perturbation.
3. *Contour packing*: computes the (x, y) position of every block in DFS pre-order; supports 90° rotation.
4. *Metrics*: bounding box, chip area, dead space and half-perimeter wirelength (HPWL).
5. *Validator*: checks that no two blocks overlap and that all positions are legal.
6. *Exporters*: SVG picture of the floorplan and HotSpot `.flp` file (input for the thermal simulator).
7. *Random-search demo*: swaps and rotations that keep only improvements (precursor of simulated annealing).
8. *Test suite*: ___ unit and integration tests, no external framework.

*Results on ami33 (33 blocks):* initial dead space ___ %, after ___ iterations of random search ___ %; HPWL ___ → ___. (Insert the two SVG images.)

**Next steps.** Simulated annealing (weeks 3–4), grid thermal model and HotSpot validation (weeks 5–6), benchmarks and short paper (weeks 7–8).

---

## Guion del video (5 min, en inglés)

Cada uno graba **su propio** video. Todos cubren el proyecto completo y cada uno profundiza en su parte.

| Tiempo | Contenido | Frases de apoyo |
|---|---|---|
| 0:00–0:40 | Problema | *"Heat is one of the main limits in chip design today, and chiplets make it worse. A floorplanner decides where each block goes."* |
| 0:40–1:40 | Enfoque | *"We represent the floorplan with a B\*-tree: the left child goes to the right of its parent, the right child goes on top. We will optimize it with simulated annealing and add a thermal model validated against HotSpot."* |
| 1:40–3:40 | **Tu parte + demo** | Ismael: el árbol y `pack` (muestra tu diagrama). Eimi: el lector y la integración (muestra la terminal leyendo ami33). Jose: métricas, validador y SVG (muestra el antes y después). |
| 3:40–4:30 | Resultados | Muestra el SVG inicial y el final y los números de espacio muerto; muestra los tests pasando. |
| 4:30–5:00 | Próximos pasos | *"Next: simulated annealing, the thermal grid model, and a short paper comparing with published results."* |

Graben la pantalla con OBS o la Xbox Game Bar (Win+G) y practiquen una vez antes de grabar.
