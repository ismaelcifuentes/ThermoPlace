# Validación del modelo térmico contra HotSpot (J15, J17)

**Pregunta:** ¿qué tan simple puede ser nuestro modelo térmico (grilla N × N con Gauss-Seidel, `src/Thermal.cpp`) sin perder precisión frente a HotSpot?

**Respuesta (corrido con v1.0, 24 nov):** bastante simple. Con un disipador de dos partes (`heatSinkH` por m² más una resistencia fija `packageR`, I15) y **un solo par de valores para los dos benchmarks**, h = 1,4·10⁵ W/(m²·K) y R = 0,20 K/W, la temperatura de cada bloque queda a **0,53 °C de HotSpot en promedio** con una grilla de 16 × 16 (0,45 °C en ami33 y 0,61 °C en ami49), y el cálculo tarda menos de 0,4 ms. Con potencia mucho más desigual (0,1–10 W/mm², J17) los mismos valores siguen siendo los mejores, pero el error sube a 1,9 °C en ami33 y 3,6 °C en ami49, donde el modelo subestima la Tmax en unos 5 °C (sección 6). En HotSpot, el peso térmico baja la Tmax de ami33 en 3,7 °C, pero en ami49 la sube (sección 7).

**Todas las tablas de las secciones 2 a 7 se corrieron con la etiqueta `v1.0`** (commit `3a1ca55`, penalización de outline lineal, J18). Los resultados anteriores, con la penalización cuadrática, quedan en la sección 8 como comparación.

## 1. Qué se comparó

| | Valor |
|---|---|
| Código | Etiqueta `v1.0` (`git checkout v1.0`) |
| Floorplans | Mejor floorplan de `--sa --outline-weight 5`, semillas 1 a 5, en ami33 y ami49 (10 floorplans). Con v1.0 caben los 10: espacio muerto medio 11,72 % (ami33) y 12,07 % (ami49) |
| Potencia | J7: semilla 1, densidad uniforme de 0,5 a 2 W/mm². Total: ami33 1,61 W, ami49 50,5 W |
| Nuestro modelo | `solveThermal` + `blockTemperatures` (I14), `dieIsOutline = true`, `siliconK = 130`, resto de `ThermalParams` por defecto |
| HotSpot | v7.0 (commit `f18831e`), **modo bloque**, estado estacionario, `examples/example1/example.config` sin cambios (`-k_chip 130`) |
| Error | Para cada bloque, \|T_nuestro − T_HotSpot\|. **MAE** = media sobre los bloques de un floorplan. **MAE conjunto** = media de los MAE de los 10 floorplans. **Error máx.** = el peor bloque. **Error en Tmax** = Tmax nuestra − Tmax de HotSpot |

Ver `docs/HOTSPOT.md` para instalar HotSpot, y `tools/README.md` para repetir todo:

```bash
git checkout v1.0 && ./build.sh
bash tools/run_j15.sh calibrate            # sección 2
bash tools/run_j15.sh grids 1.4e5 0.20     # sección 3
```

### Relleno del espacio muerto

HotSpot pide un floorplan **sin huecos** ("There cannot be any empty spaces in the chip", `README_archive/README-2.0` de HotSpot). Nuestro `.flp` solo trae los bloques, y nuestro modelo usa todo el outline como die (I13). Para comparar el mismo chip se agregaron **bloques de relleno con 0 W** que cubren el die completo: se corta el die en franjas verticales en cada borde de bloque, y cada hueco de cada franja es un relleno (57 a 70 rellenos en ami33, 127 a 150 en ami49; el die queda cubierto al 100 %). Lo hace `tools/j15_tool.cpp`.

## 2. Calibración del disipador

Nuestro modelo tiene dos parámetros de disipador (I15, `include/thermoplace/Thermal.hpp`):

- `heatSinkH` (W/(m²·K)): paso del calor del die al spreader, **por m² de die**.
- `packageR` (K/W): spreader, disipador y aire, **una sola resistencia para todo el chip**.

Se probaron todos los pares de `heatSinkH` de 10⁴ a 10⁷ (20 valores por década) y `packageR` de 0 a 1 K/W (de 0,02 en 0,02), con grilla 32, y se eligió el par con menor MAE conjunto.

| Calibración | `heatSinkH` | `packageR` (K/W) | MAE ami33 (°C) | MAE ami49 (°C) | MAE conjunto (°C) |
|---|---|---|---|---|---|
| **Conjunta (la elegida)** | 1,41·10⁵ | 0,20 | 0,39 | 0,58 | **0,49** |
| Conjunta, redondeada (por defecto en v1.0) | **1,4·10⁵** | **0,20** | 0,45 | 0,60 | 0,53 |
| Solo ami33 | 1,78·10⁵ | 0,90 | 0,13 | — | — |
| Solo ami49 | 1,26·10⁵ | 0,18 | — | 0,54 | — |
| Valores provisionales de I15 | 1,12·10⁵ | 0,28 | 2,38 | 6,50 | 4,44 |

Para cada `heatSinkH`, el mejor `packageR` y su MAE conjunto (grilla 32):

| `heatSinkH` | Mejor `packageR` | MAE conjunto (°C) |
|---|---|---|
| 7,94·10⁴ | 0,06 | 3,34 |
| 1,00·10⁵ | 0,12 | 1,93 |
| 1,26·10⁵ | 0,18 | 0,88 |
| **1,41·10⁵** | **0,20** | **0,49** |
| 1,58·10⁵ | 0,22 | 0,54 |
| 2,00·10⁵ | 0,24 | 1,20 |
| 2,82·10⁵ | 0,28 | 2,06 |

El mínimo es claro en `heatSinkH`: un 10 % de diferencia sube el error poco, pero un factor 2 lo lleva a 1,2–2 °C. **Con v1.0 el mejor par es el mismo que con la penalización cuadrática** (sección 8), así que los valores por defecto (`heatSinkH = 1.4e5`, `packageR = 0.20`) no cambian.

## 3. Errores por grilla (h = 1,4·10⁵, R = 0,20)

Medias sobre las 5 semillas; el error máximo es el peor bloque de las 5.

| Benchmark | Grilla | MAE (°C) | Error máx. (°C) | Error en Tmax (°C) | Tiempo (ms) |
|---|---|---|---|---|---|
| ami33 | 8 | 0,45 | 1,01 | +0,55 | 0,05 |
| ami33 | 16 | 0,45 | 1,05 | +0,57 | 0,36 |
| ami33 | 32 | 0,45 | 1,06 | +0,58 | 3,16 |
| ami33 | 64 | 0,45 | 1,05 | +0,58 | 26,83 |
| ami49 | 8 | 0,82 | 3,02 | −1,15 | 0,03 |
| ami49 | 16 | 0,61 | 2,00 | −0,90 | 0,15 |
| ami49 | 32 | 0,60 | 2,15 | −0,85 | 1,17 |
| ami49 | 64 | 0,60 | 2,14 | −0,85 | 9,57 |

MAE conjunto por grilla: 8 → 0,63 °C, 16 → 0,53 °C, 32 → 0,53 °C, 64 → 0,53 °C.

Tmax de HotSpot (media de las 5 semillas): ami33 53,5 °C, ami49 68,8 °C. Una corrida de HotSpot tarda entre 16 y 31 ms en ami33 y entre 99 y 177 ms en ami49, según la carga de la PC (proceso completo, con arranque y lectura de archivos, así que la comparación de tiempos es aproximada).

## 4. Conclusión

1. **Disipador de dos partes.** Con solo `heatSinkH` (por m²), ningún valor servía para ami33 y ami49 a la vez: el mejor dejaba 4,7 °C de error medio (sección 9). Con la resistencia fija `packageR`, un solo par (1,4·10⁵; 0,20) deja 0,53 °C. La parte fija representa el spreader y el disipador de HotSpot, que son mucho más grandes que los dos dies y no escalan con su área.
2. **Grilla.** N = 16 basta: el MAE conjunto es 0,53 °C con N = 16, igual que con N = 64, mientras el cálculo pasa de 0,15–0,36 ms a 10–27 ms. N = 8 sirve en ami33, pero en ami49 el peor bloque llega a 3,0 °C de error.
3. **Precisión.** Con N = 16, el error en Tmax es de +0,6 °C en ami33 y −0,9 °C en ami49, y el peor bloque de los 10 floorplans tiene 2,0 °C de error.

## 5. Límites

- Solo dos benchmarks y una sola semilla de potencia (semilla 1). La calibración se hizo y se evaluó sobre los mismos 10 floorplans: no hay un conjunto aparte de prueba. Con potencia mucho más desigual (0,1–10 W/mm²) el error crece, sobre todo en ami49 (sección 6).
- La calibración usa floorplans de SA sin término térmico. Los floorplans con peso térmico se revisan en la sección 7.
- Se compara con HotSpot en modo bloque, que también es un modelo (una temperatura por bloque); no es una medición.
- La configuración de HotSpot es la de `example.config` (`-r_convec 0.1`). Con otro encapsulado, los valores calibrados cambian.

## 6. Potencia muy desigual (J17)

**Pregunta:** ¿el modelo sigue cerca de HotSpot cuando la potencia es muy desigual? Con 0,1–10 W/mm² el peso térmico tiene mucho más efecto (E14), así que esos resultados dependen de que el modelo sea preciso con esa potencia.

**Qué se hizo:** los mismos 10 floorplans de las secciones 2 y 3, con la potencia de J7 pero con densidades de **0,1 a 10 W/mm²** (semilla 1; total ami33 6,91 W, ami49 220,1 W, unas 4,3 veces más que con 0,5–2 W/mm²), y **sin recalibrar**: h = 1,4·10⁵, R = 0,20.

```bash
J15_OUT=build/j17 POWER_MIN=0.1 POWER_MAX=10 bash tools/run_j15.sh validate 1.4e5 0.20
```

Como la potencia total es mayor, el chip se calienta más y el error en °C crece aunque el modelo sea igual de bueno. Por eso se agrega el **error relativo**: el MAE dividido por la subida media de temperatura de los bloques en HotSpot (temperatura menos 45 °C de ambiente).

| Benchmark | Potencia (W/mm²) | Grilla | MAE (°C) | MAE / subida (%) | Error máx. (°C) | Error en Tmax (°C) | Tmax HotSpot (°C) |
|---|---|---|---|---|---|---|---|
| ami33 | 0,5–2 | 16 | 0,45 | 6,0 | 1,05 | +0,57 | 53,5 |
| ami33 | **0,1–10** | 16 | **1,89** | **5,9** | 4,56 | +2,72 | 82,2 |
| ami33 | 0,1–10 | 32 | 1,91 | 5,9 | 4,64 | +2,77 | 82,2 |
| ami49 | 0,5–2 | 16 | 0,61 | 3,4 | 2,00 | −0,90 | 68,8 |
| ami49 | **0,1–10** | 16 | **3,56** | **4,7** | 11,87 | −5,33 | 156,7 |
| ami49 | 0,1–10 | 32 | 3,44 | 4,5 | 12,01 | −5,08 | 156,7 |
| ami49 | 0,1–10 | 64 | 3,39 | 4,5 | 11,49 | −5,08 | 156,7 |

MAE conjunto con 0,1–10 W/mm²: 2,72 °C (N = 16), 2,67 °C (N = 32).

**Recalibración con esta potencia** (`calibrate`, mismo barrido 2D): el mejor par conjunto es otra vez **h = 1,41·10⁵, R = 0,20** (MAE conjunto 2,50 °C; ami33 1,65 °C, ami49 3,35 °C). Calibrando solo ami49, lo mejor es h = 1,26·10⁵, R = 0,18, con 3,01 °C.

**Resultado:**

1. **La calibración no depende de la potencia.** El mejor (h, R) es el mismo con 0,5–2 y con 0,1–10 W/mm².
2. **ami33: el error crece solo porque hay más potencia.** En relativo queda igual (6,0 % → 5,9 %). En °C pasa de 0,45 a 1,89, por encima del ~1 °C buscado.
3. **ami49: la potencia desigual sí empeora el modelo.** El error relativo sube de 3,4 % a 4,7 % (N = 16), el peor bloque llega a 11,9 °C de error, y el modelo **subestima la Tmax en unos 5 °C**. No es por la grilla (con N = 64 sigue en −5,1 °C) ni por la calibración (calibrando solo ami49, el MAE no baja de 3,0 °C). Es un límite del modelo con bloques muy calientes junto a bloques casi apagados. No sabemos la causa exacta; una posibilidad es que nuestro camino vertical (un solo `heatSinkH` por m²) reparta el calor de un bloque caliente más que las capas de HotSpot (interfaz, spreader y disipador por separado).

## 7. ¿El peso térmico baja la Tmax de verdad? (J17 con E14)

Se generaron con el programa los floorplans de peso térmico 0 y 0,25, con 0,1–10 W/mm², semillas 1 a 5, y se pasaron por HotSpot (con relleno, `tools/j15_tool.cpp` en modo `filled` con `POWER_MIN=0.1 POWER_MAX=10`):

```bash
build/thermoplace benchmarks/mcnc/ami33.block --sa --outline-weight 5 --thermal \
    --thermal-weight 0.25 --power-min 0.1 --power-max 10 --seed 1
```

Tmax por bloque (la temperatura del bloque más caliente, en los dos modelos), media de las 5 semillas:

| Benchmark | Tmax nuestra, peso 0 → 0,25 | Tmax HotSpot, peso 0 → 0,25 | Baja en los dos | Misma dirección | Caben (peso 0 / 0,25) |
|---|---|---|---|---|---|
| ami33 | 84,96 → 79,39 °C (**−5,57**) | 82,19 → 78,50 °C (**−3,69**) | 5/5 | 5/5 | 5/5 / 5/5 |
| ami49 | 151,60 → 153,02 °C (**+1,42**) | 156,68 → 160,23 °C (**+3,55**) | 0/5 | 4/5 | 5/5 / 3/5 |

La Tmax por celda que reporta el programa (`max_temp_c`) es 85,63 → 79,92 °C en ami33 y 158,80 → 159,15 °C en ami49, igual que en la tabla E14 de `docs/RESULTS.md`.

- **ami33: la mejora es real.** HotSpot también baja la Tmax en las 5 semillas, 3,7 °C en promedio. Nuestro modelo la exagera en unos 1,9 °C.
- **ami49: el peso 0,25 no baja la Tmax; HotSpot dice que la sube 3,6 °C.** Los dos modelos coinciden en la dirección en 4 de 5 semillas, así que no es un error del modelo rápido: con v1.0 el peso térmico de verdad no ayuda en ami49. Con la penalización cuadrática (sección 8) la Tmax de ami49 sí bajaba, pero ninguno de esos floorplans de peso 0,25 cabía en el outline: el die era más grande y por eso más frío. Con la penalización lineal 3 de 5 caben y esa ganancia desaparece, como anticipó Eimi.

## 8. Comparación: antes de v1.0 (penalización cuadrática)

Mismos experimentos, corridos entre el 10 y el 17 nov con la penalización de outline cuadrática (antes de J18). Con esa penalización cabían 5/5 floorplans de ami33 (15,63 % de espacio muerto medio) y 3/5 de ami49 (11,90 %).

| Resultado | Cuadrática | Lineal (v1.0) |
|---|---|---|
| Mejor (h, R) con 0,5–2 W/mm² | 1,41·10⁵; 0,20 | 1,41·10⁵; 0,20 |
| MAE conjunto con (1,4·10⁵; 0,20), N = 32 | 0,53 °C | 0,53 °C |
| MAE conjunto, N = 16 | 0,55 °C | 0,53 °C |
| ami33 / ami49: MAE con N = 16 | 0,44 / 0,66 °C | 0,45 / 0,61 °C |
| ami33 / ami49: error en Tmax, N = 16 | +0,46 / −1,36 °C | +0,57 / −0,90 °C |
| Peor bloque, N = 16 | 2,33 °C | 2,00 °C |
| Mejor (h, R) con 0,1–10 W/mm² | 1,41·10⁵; 0,20 | 1,41·10⁵; 0,20 |
| 0,1–10 W/mm², ami33 / ami49: MAE con N = 16 | 1,87 / 3,73 °C | 1,89 / 3,56 °C |
| 0,1–10 W/mm², ami49: error en Tmax, N = 16 | −7,49 °C | −5,33 °C |
| Peso 0 → 0,25, ami33: Tmax nuestra / HotSpot | −4,20 / −3,54 °C (5/5) | −5,57 / −3,69 °C (5/5) |
| Peso 0 → 0,25, ami49: Tmax nuestra / HotSpot | −0,58 / −1,84 °C (1/5) | +1,42 / +3,55 °C (0/5) |
| Peso 0,25, ami49: caben | 0/5 | 3/5 |

(En la comparación de pesos con la penalización cuadrática se usó el `main` del 17 nov, ya con I16.)

## 9. Primera versión (5 nov): solo `heatSinkH`

Con el modelo anterior (sin `packageR`, `siliconK = 100` y HotSpot con `-k_chip 100`), el `heatSinkH` por defecto (4·10⁴) sobrestimaba la temperatura en 10 a 18 °C. El mejor valor de cada benchmark era distinto (1,26·10⁵ en ami33 con MAE 0,59 °C; 6,31·10⁴ en ami49 con MAE 1,18 °C), y el mejor valor común (7,94·10⁴) dejaba 4,7 °C de MAE. Sin relleno, HotSpot daba una Tmax 2,7 a 4 °C más alta. Esto llevó a agregar `packageR` (I15).
