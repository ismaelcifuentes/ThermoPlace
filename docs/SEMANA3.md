# Semana 3 (22–28 oct): ajuste de SA y comparación

**Estado al 28 de octubre.** SA funciona en `main` (PR #7 a #10): en ami33, semilla 1, el espacio muerto baja de 24,2 % (búsqueda aleatoria, 20 000 iteraciones) a 14,85 % y el chip cabe en el outline. La semana se cierra **hoy, 28 de octubre**.

| Persona | Tareas | Archivos | Prueba |
|---|---|---|---|
| **Ismael** | I9 mejor solución factible, I10 `moveSubtree` (ya en el PR `ismael/sa-tuning`) | `src/Annealer.cpp`, `src/BStarTree.cpp` | `thermoplace_tests ismael` (19/19) |
| **Eimi** | E8b opciones nuevas, E10 tabla de resultados | `app/main.cpp`, `docs/RESULTS.md` (nuevo) | correr el programa |
| **Jose** | J10 figuras finales y revisión de HPWL | `results/` (no se sube), `docs/RESULTS.md` | revisar SVG |

## Ismael (hecho, en revisión)

- **I9.** `SAParams::feasible` (opcional): si se define, `anneal` devuelve la mejor solución **que cumple** la condición (por ejemplo, "cabe en el outline"), aunque haya visto otra con menor costo que no cabe. `SAResult::feasibleFound` dice si encontró alguna.
- **I10.** `BStarTree::moveSubtree`: mueve un nodo **con todo su subárbol** a un espacio libre. `anneal` ahora mezcla mover hojas y mover subárboles, así explora árboles más distintos.
- Las semillas dan resultados **distintos** a los de antes de este PR (cambió el orden de los números aleatorios). Por eso las corridas finales se hacen **después** de unirlo.

Pruebas en ami33/ami49 (5 semillas, alpha 0.5, outline-weight 5, con `feasible`):

| Benchmark | Ajustes | Cabe en outline | Espacio muerto medio | Tiempo medio |
|---|---|---|---|---|
| ami33 | por defecto (cooling 0.95, 10n movidas) | 5/5 | 15,6 % | 0,2 s |
| ami33 | cooling 0.98, 2000 movidas por temperatura | 5/5 | 12,8 % | 3,5 s |
| ami49 | por defecto | 3/5 | 11,9 % | 0,7 s |
| ami49 | cooling 0.98, 2000 movidas por temperatura | 2/5 | 10,8 % | 7,0 s |

ami49 es difícil: su outline (5336 × 7673) deja solo 13,4 % de espacio libre. Más esfuerzo baja el espacio muerto pero no garantiza que quepa; queda como pregunta para el informe.

## Eimi

### E8b. Opciones nuevas en `main.cpp` (30 min, después de unir `ismael/sa-tuning`)

- `--moves-per-temp N` → `SAParams::movesPerTemp` (0 = automático, 10 × bloques).
- Si `--outline-weight` es mayor que 0, define `p.feasible = [](const Design& x) { return computeMetrics(x).fitsOutline; };` e imprime si encontró una solución que cabe (`res.feasibleFound`).

### E10. Tabla de resultados (1–2 h) — `docs/RESULTS.md`

Corre, con las mismas semillas 1 a 5, sobre ami33 y ami49:

```bat
build\thermoplace.exe benchmarks\mcnc\ami33.block --iters 20000 --seed 1
build\thermoplace.exe benchmarks\mcnc\ami33.block --sa --outline-weight 5 --seed 1
build\thermoplace.exe benchmarks\mcnc\ami33.block --sa --outline-weight 5 --cooling 0.98 --moves-per-temp 2000 --seed 1
```

Todo queda en `results\runs.csv`. En `docs/RESULTS.md` pon una tabla con media y mejor valor de espacio muerto, HPWL, cuántas veces cabe y el tiempo, por método. Agrega una fila con el resultado **publicado** del B\*-tree original (Chang et al., DAC 2000) y del Fast-SA (Chen y Chang, TCAD 2006) para ami33 y ami49, **copiando el valor y la tabla del paper de donde sale**.

## Jose

### J10. Figuras finales y revisión de HPWL (1 h)

- Con las corridas de Eimi, guarda el SVG del mejor floorplan y la curva de convergencia de ami33 y ami49 (los genera el programa en `results\`). Revisa que se lean bien; si hace falta, mejora `exportSVG` (por ejemplo, dibujar el outline del benchmark además del contorno del chip).
- Revisa en Chen y Chang (TCAD 2006) cómo definen HPWL (¿centro de los bloques? ¿incluye terminales?) y anota en `docs/RESULTS.md` si es igual a la nuestra. Si no es igual, la comparación de HPWL con el paper no es válida y hay que decirlo.

## Siguiente: semana 4 (29 oct – 4 nov) — modelo térmico

Ismael prepara los contratos (`Thermal.hpp`, tests y `docs/SEMANA4.md`) el 29 de octubre: grilla N × N sobre el chip, potencia por celda y temperatura por difusión de calor (Jacobi / Gauss-Seidel).
