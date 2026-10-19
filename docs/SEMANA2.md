# Semana 2 (15–21 de octubre): simulated annealing

**Meta de la semana:** reemplazar la búsqueda aleatoria por **simulated annealing (SA)** y poder medir cada corrida.
Al terminar, `build\thermoplace_tests.exe week2` debe dar **11/11** y el programa debe aceptar `--sa`.

Como en la semana 1, los `.hpp` nuevos ya definen qué recibe y qué devuelve cada función, y cada persona tiene sus propios tests. **Las tres partes se hacen en paralelo**: nadie necesita esperar a nadie para que sus tests pasen.

| Persona | Archivos | Tareas | Prueba |
|---|---|---|---|
| **Ismael** | `src/BStarTree.cpp`, `src/Annealer.cpp` | I7 `moveNode`, I8 `anneal` | `thermoplace_tests ismael` |
| **Jose** | `src/Cost.cpp`, `src/Exporters.cpp` | J8 función de costo, J9 gráfica de convergencia | `thermoplace_tests jose` |
| **Eimi** | `src/Report.cpp`, `app/main.cpp` | E9 reportes CSV, E8 opciones `--sa` en el programa (al final, integra) | `thermoplace_tests eimi` |

Contratos nuevos: `include/thermoplace/Annealer.hpp`, `Cost.hpp`, `Report.hpp` y `exportConvergenceSVG` en `Exporters.hpp`. **No los cambien** sin avisar al grupo.

## Antes de empezar (los tres)

```bat
git checkout main
git pull
build.bat
build\thermoplace_tests.exe week2
```

Que fallen los tests de `week2` es lo esperado. Luego cada uno crea su rama: `ismael/anneal`, `jose/cost`, `eimi/report`.

## Ismael

### I7. `moveNode` (unos 20 min) — `src/BStarTree.cpp`

Versión simple: solo mueve **hojas**. Devuelve `false` (sin cambiar nada) si `node` es la raíz, si `node == newParent`, si `node` tiene hijos o si el espacio pedido de `newParent` ya está ocupado. Si se puede: quita el nodo de su padre actual (pon `-1` en `left` o `right`), ponlo en el espacio libre y actualiza `parent`.
Prueba: `thermoplace_tests bstar_extra`.

### I8. `anneal` (2–3 h) — `src/Annealer.cpp`

1. Empaqueta y mide el costo inicial. Guarda el mejor árbol (una copia con `=`) **y** las rotaciones de los bloques (viven en el `Design`, no en el árbol).
2. **Temperatura inicial:** prueba `2n` movimientos al azar, promedia los que empeoran (`delta > 0`) y usa `T0 = -promedio / ln(initialAcceptProb)`.
3. **Bucle:** en cada temperatura prueba `movesPerTemp` movimientos (rotar, `swapBlocks` o `moveNode`, según los pesos). Acepta si `delta <= 0` o si `random < exp(-delta / T)` (Metropolis). Si se rechaza una rotación, deshazla.
4. Al final de cada temperatura agrega un `SATracePoint` y enfría: `T = cooling * T`. Para cuando `T < minTempRatio * T0`.
5. Al terminar, deja en `tree` el mejor árbol y en `d` sus rotaciones y posiciones.

Prueba: `thermoplace_tests anneal` (usa un costo simple, el área; no depende de Jose).

## Jose

### J8. Función de costo (unos 40 min) — `src/Cost.cpp`

Las fórmulas están en `include/thermoplace/Cost.hpp`:
- `makeNorm`: `area = chipArea`, `hpwl = hpwl`; si alguno es `<= 0`, usa 1.
- `outlinePenalty`: 0 si no hay outline o si el chip cabe; si no, `(exW/outlineW)^2 + (exH/outlineH)^2`.
- `computeCost`: `alpha * area/norm.area + (1 - alpha) * hpwl/norm.hpwl + outline * outlinePenalty`.

Prueba: `thermoplace_tests cost`.

### J9. Gráfica de convergencia (1 h) — `exportConvergenceSVG` al final de `src/Exporters.cpp`

SVG de líneas: eje x = paso de temperatura, eje y = costo. Dos `<polyline>`: costo actual y mejor costo (colores distintos), los valores mínimo y máximo escritos en el eje y, una leyenda y el título. Devuelve `false` si el trace está vacío.
Prueba: `thermoplace_tests convergence`. Abre `results\test_convergence.svg` en el navegador para verla.

## Eimi

### E9. Reportes CSV (unos 40 min) — `src/Report.cpp`

- `writeTraceCSV`: sobrescribe el archivo; encabezado exacto `step,temperature,current_cost,best_cost,accept_rate` y una línea por punto.
- `appendRunCSV`: agrega una línea; si el archivo no existe o está vacío, primero escribe el encabezado de `Report.hpp`. `fits_outline` va como `1` o `0`.
- Para saber si el archivo está vacío: ábrelo con `std::ifstream` y mira `peek()`. **No uses `std::filesystem`** (falla con MinGW viejo).

Prueba: `thermoplace_tests report`.

### E8. Opciones de SA en el programa (1–2 h) — `app/main.cpp`, cuando Ismael y Jose hagan merge

Nuevas opciones: `--sa` (usar SA en vez de búsqueda aleatoria), `--alpha A` (0.5), `--outline-weight W` (0), `--cooling C` (0.95), `--runs-csv FILE` (por defecto `results/runs.csv`).
Con `--sa`: `CostNorm` desde las métricas iniciales, `CostWeights` desde las opciones, un lambda `cost` que llame a `computeCost(computeMetrics(x), x, w, norm)`, y `anneal`. Después escribe `results/<bench>_trace.csv`, `results/<bench>_convergence.svg`, el SVG/FLP `best`, y una línea en el CSV de corridas (también para `random`, con `method = "random"`). Mide el tiempo con `std::chrono::steady_clock`.

Ejemplo: `build\thermoplace.exe benchmarks\mcnc\ami33.block --sa --alpha 0.5 --seed 1`

## Cierre de la semana

- [ ] Los tres PR integrados en `main`, `thermoplace_tests core` y `week2` en verde
- [ ] Corrida de ami33 con `--sa` para las semillas 1 a 5, en `results/runs.csv`
- [ ] Comparación rápida en el grupo: ¿cuánto mejora SA a la búsqueda aleatoria?
