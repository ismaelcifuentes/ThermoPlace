# Validación del modelo térmico contra HotSpot (J15)

**Pregunta:** ¿qué tan simple puede ser nuestro modelo térmico (grilla N × N con Gauss-Seidel, `src/Thermal.cpp`) sin perder precisión frente a HotSpot?

**Respuesta (10 nov):** bastante simple. Con un disipador de dos partes (`heatSinkH` por m² más una resistencia fija `packageR`, I15) y **un solo par de valores para los dos benchmarks**, h = 1,4·10⁵ W/(m²·K) y R = 0,20 K/W, la temperatura de cada bloque queda a **0,55 °C de HotSpot en promedio** con una grilla de 16 × 16 (0,44 °C en ami33 y 0,66 °C en ami49). Con N = 16 el cálculo tarda menos de 0,5 ms, y el error es prácticamente el mismo que con N = 64.

## 1. Qué se comparó

| | Valor |
|---|---|
| Floorplans | Mejor floorplan de `--sa --outline-weight 5`, semillas 1 a 5, en ami33 y ami49 (10 floorplans) |
| Potencia | J7: semilla 1, densidad uniforme de 0,5 a 2 W/mm². Total: ami33 1,61 W, ami49 50,5 W |
| Nuestro modelo | `solveThermal` + `blockTemperatures` (I14), `dieIsOutline = true`, `siliconK = 130`, resto de `ThermalParams` por defecto |
| HotSpot | v7.0 (commit `f18831e`), **modo bloque**, estado estacionario, `examples/example1/example.config` sin cambios (`-k_chip 130`) |
| Error | Para cada bloque, \|T_nuestro − T_HotSpot\|. **MAE** = media sobre los bloques de un floorplan. **MAE conjunto** = media de los MAE de los 10 floorplans. **Error máx.** = el peor bloque. **Error en Tmax** = Tmax nuestra − Tmax de HotSpot |

Ver `docs/HOTSPOT.md` para instalar HotSpot, y `tools/README.md` para repetir todo con `bash tools/run_j15.sh calibrate`.

### Relleno del espacio muerto

HotSpot pide un floorplan **sin huecos** ("There cannot be any empty spaces in the chip", `README_archive/README-2.0` de HotSpot). Nuestro `.flp` solo trae los bloques, y nuestro modelo usa todo el outline como die (I13). Para comparar el mismo chip se agregaron **bloques de relleno con 0 W** que cubren el die completo: se corta el die en franjas verticales en cada borde de bloque, y cada hueco de cada franja es un relleno (54 a 70 rellenos en ami33, 128 a 150 en ami49; el die queda cubierto al 100 %). Lo hace `tools/j15_tool.cpp`.

## 2. Calibración del disipador

Nuestro modelo tiene dos parámetros de disipador (I15, `include/thermoplace/Thermal.hpp`):

- `heatSinkH` (W/(m²·K)): paso del calor del die al spreader, **por m² de die**.
- `packageR` (K/W): spreader, disipador y aire, **una sola resistencia para todo el chip**.

Se probaron todos los pares de `heatSinkH` de 10⁴ a 10⁷ (20 valores por década) y `packageR` de 0 a 1 K/W (de 0,02 en 0,02), con grilla 32, y se eligió el par con menor MAE conjunto.

| Calibración | `heatSinkH` | `packageR` (K/W) | MAE ami33 (°C) | MAE ami49 (°C) | MAE conjunto (°C) |
|---|---|---|---|---|---|
| **Conjunta (la elegida)** | 1,41·10⁵ | 0,20 | 0,38 | 0,60 | **0,49** |
| Conjunta, redondeada | **1,4·10⁵** | **0,20** | 0,44 | 0,62 | 0,53 |
| Solo ami33 | 1,78·10⁵ | 0,90 | 0,11 | — | — |
| Solo ami49 | 1,26·10⁵ | 0,18 | — | 0,54 | — |
| Valores provisionales de I15 | 1,12·10⁵ | 0,28 | 2,37 | 6,38 | 4,38 |

Para cada `heatSinkH`, el mejor `packageR` y su MAE conjunto (grilla 32):

| `heatSinkH` | Mejor `packageR` | MAE conjunto (°C) |
|---|---|---|
| 7,94·10⁴ | 0,08 | 3,29 |
| 1,00·10⁵ | 0,12 | 1,95 |
| 1,26·10⁵ | 0,18 | 0,88 |
| **1,41·10⁵** | **0,20** | **0,49** |
| 1,58·10⁵ | 0,22 | 0,55 |
| 2,00·10⁵ | 0,24 | 1,21 |
| 2,82·10⁵ | 0,28 | 2,06 |

El mínimo es claro en `heatSinkH`: un 10 % de diferencia sube el error poco, pero un factor 2 lo lleva a 1,2–2 °C.

**Valores por defecto propuestos para `Thermal.hpp`: `heatSinkH = 1.4e5`, `packageR = 0.20`.**

## 3. Errores por grilla (h = 1,4·10⁵, R = 0,20)

Medias sobre las 5 semillas; el error máximo es el peor bloque de las 5.

| Benchmark | Grilla | MAE (°C) | Error máx. (°C) | Error en Tmax (°C) | Tiempo (ms) |
|---|---|---|---|---|---|
| ami33 | 8 | 0,42 | 0,91 | +0,44 | 0,06 |
| ami33 | 16 | 0,44 | 0,88 | +0,46 | 0,42 |
| ami33 | 32 | 0,44 | 0,90 | +0,47 | 3,97 |
| ami33 | 64 | 0,44 | 0,90 | +0,47 | 32,84 |
| ami49 | 8 | 0,80 | 3,59 | −1,62 | 0,04 |
| ami49 | 16 | 0,66 | 2,33 | −1,36 | 0,25 |
| ami49 | 32 | 0,62 | 2,32 | −1,27 | 2,14 |
| ami49 | 64 | 0,62 | 2,38 | −1,25 | 17,61 |

MAE conjunto por grilla: 8 → 0,61 °C, 16 → 0,55 °C, 32 → 0,53 °C, 64 → 0,53 °C.

Tmax de HotSpot (media de las 5 semillas): ami33 53,6 °C, ami49 69,3 °C. HotSpot tarda unos 16 ms por floorplan en ami33 y 99 ms en ami49 (proceso completo, con arranque y lectura de archivos, así que la comparación de tiempos es aproximada).

## 4. Conclusión

1. **Disipador de dos partes.** Con solo `heatSinkH` (por m²), ningún valor servía para ami33 y ami49 a la vez: el mejor dejaba 4,7 °C de error medio (sección 6). Con la resistencia fija `packageR`, un solo par (1,4·10⁵; 0,20) deja 0,53 °C. La parte fija representa el spreader y el disipador de HotSpot, que son mucho más grandes que los dos dies y no escalan con su área.
2. **Grilla.** N = 16 basta: el MAE conjunto pasa de 0,55 °C (N = 16) a 0,53 °C (N = 64), mientras el cálculo pasa de 0,25–0,42 ms a 18–33 ms. N = 8 sirve en ami33, pero en ami49 el peor bloque llega a 3,6 °C de error.
3. **Precisión.** El error en Tmax es de +0,5 °C en ami33 y −1,3 °C en ami49 con N = 16. El peor bloque de los 10 floorplans tiene 2,3 °C de error.

## 5. Límites

- Solo dos benchmarks y una sola asignación de potencia (semilla 1). La calibración se hizo y se evaluó sobre los mismos 10 floorplans: no hay un conjunto aparte de prueba.
- Solo floorplans de SA sin término térmico. Los floorplans del frente de Pareto (E13) pueden tener otra distribución de calor.
- Se compara con HotSpot en modo bloque, que también es un modelo (una temperatura por bloque); no es una medición.
- La configuración de HotSpot es la de `example.config` (`-r_convec 0.1`). Con otro encapsulado, los valores calibrados cambian.

## 6. Primera versión (5 nov): solo `heatSinkH`

Con el modelo anterior (sin `packageR`, `siliconK = 100` y HotSpot con `-k_chip 100`), el `heatSinkH` por defecto (4·10⁴) sobrestimaba la temperatura en 10 a 18 °C. El mejor valor de cada benchmark era distinto (1,26·10⁵ en ami33 con MAE 0,59 °C; 6,31·10⁴ en ami49 con MAE 1,18 °C), y el mejor valor común (7,94·10⁴) dejaba 4,7 °C de MAE. Sin relleno, HotSpot daba una Tmax 2,7 a 4 °C más alta. Esto llevó a agregar `packageR` (I15).
