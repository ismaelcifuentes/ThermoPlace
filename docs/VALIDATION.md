# Validación del modelo térmico contra HotSpot (J15)

**Pregunta:** ¿qué tan simple puede ser nuestro modelo térmico (grilla N × N con Gauss-Seidel, `src/Thermal.cpp`) sin perder precisión frente a HotSpot?

**Respuesta corta (5 nov, versión preliminar):** la grilla puede ser chica. Con N = 16 el error es prácticamente el mismo que con N = 64 y el cálculo es unas 70 veces más rápido. Lo que sí importa es el disipador: con el `heatSinkH` por defecto (4·10⁴) nuestro modelo sobrestima la temperatura en 10 a 18 °C. Calibrado para cada benchmark, el error medio por bloque baja a 0,6 °C (ami33) y 1,2 °C (ami49). Pero **no hay un único `heatSinkH` que sirva para los dos**: el mejor valor común deja unos 4,7 °C de error medio.

## 1. Qué se comparó

| | Valor |
|---|---|
| Floorplans | Mejor floorplan de `--sa --outline-weight 5`, semillas 1 a 5, en ami33 y ami49 (10 floorplans) |
| Potencia | J7: semilla 1, densidad uniforme de 0,5 a 2 W/mm². Total: ami33 1,61 W, ami49 50,5 W |
| Nuestro modelo | `solveThermal` + `blockTemperatures` (I14), grillas 8, 16, 32 y 64, `dieIsOutline = true`, `siliconK = 100`, resto de `ThermalParams` por defecto |
| `heatSinkH` | Barrido de 10³ a 10⁷ W/(m²·K), 10 valores por década |
| HotSpot | v7.0 (commit `f18831e`), **modo bloque**, estado estacionario, `examples/example1/example.config` con `-k_chip 100` (para usar el mismo silicio que nosotros) |
| Error | Para cada bloque, \|T_nuestro − T_HotSpot\|. **MAE** = media sobre los bloques; **error máx.** = el peor bloque; **error en Tmax** = Tmax nuestra − Tmax de HotSpot |

Ver `docs/HOTSPOT.md` para instalar HotSpot y el formato de sus archivos.

### Relleno del espacio muerto

HotSpot pide un floorplan **sin huecos** ("There cannot be any empty spaces in the chip", `README_archive/README-2.0` de HotSpot). Nuestro `.flp` solo trae los bloques, y nuestro modelo usa todo el outline como die (I13). Para comparar el mismo chip, se agregaron **bloques de relleno con 0 W** que cubren el die completo: se corta el die en franjas verticales en cada borde de bloque, y cada hueco de cada franja es un relleno (54 a 70 rellenos en ami33, 128 a 150 en ami49; el die queda cubierto al 100 %).

Los resultados principales son **con relleno**. La sección 5 muestra qué pasa sin relleno.

## 2. Errores por grilla (con relleno, `heatSinkH` calibrado por benchmark)

`heatSinkH` elegido con la grilla 32: ami33 **1,26·10⁵**, ami49 **6,31·10⁴** W/(m²·K). Medias sobre las 5 semillas; el error máximo es el peor bloque de las 5.

| Benchmark | Grilla | MAE (°C) | Error máx. (°C) | Error en Tmax (°C) | Tiempo (ms) |
|---|---|---|---|---|---|
| ami33 | 8 | 0,57 | 1,15 | +0,60 | 0,04 |
| ami33 | 16 | 0,59 | 1,11 | +0,63 | 0,35 |
| ami33 | 32 | 0,59 | 1,14 | +0,64 | 2,81 |
| ami33 | 64 | 0,59 | 1,14 | +0,64 | 24,95 |
| ami49 | 8 | 1,47 | 5,06 | +2,29 | 0,03 |
| ami49 | 16 | 1,20 | 4,52 | +2,77 | 0,24 |
| ami49 | 32 | 1,18 | 4,60 | +2,93 | 2,10 |
| ami49 | 64 | 1,18 | 4,65 | +2,96 | 17,28 |

Tiempo de HotSpot (proceso completo, con relleno): unos 25 ms en ami33 y entre 120 y 220 ms en ami49.

Tmax de HotSpot (con relleno): ami33 54,2 °C de media (53,7 a 54,8), ami49 70,1 °C (69,7 a 70,4).

## 3. Calibración de `heatSinkH`

MAE con la grilla 32 (con relleno) según `heatSinkH`:

| `heatSinkH` (W/(m²·K)) | MAE ami33 (°C) | MAE ami49 (°C) | MAE de los dos (°C) |
|---|---|---|---|
| 3,98·10⁴ (por defecto) | 18,07 | 10,08 | 14,07 |
| 5,01·10⁴ | 12,84 | 4,24 | 8,54 |
| 6,31·10⁴ | 8,67 | **1,18** | 4,92 |
| 7,94·10⁴ | 5,35 | 4,05 | **4,70** |
| 1,00·10⁵ | 2,70 | 6,96 | 4,83 |
| 1,26·10⁵ | **0,59** | 9,26 | 4,92 |
| 1,58·10⁵ | 1,11 | 11,09 | 6,10 |

- El **mejor valor común** es **7,94·10⁴**, con 4,7 °C de MAE. Con ese valor, ami33 queda 5,7 °C más caliente que HotSpot en Tmax y ami49 2,6 °C más frío.
- El mejor valor de cada benchmark es distinto: **1,26·10⁵** para ami33 y **6,31·10⁴** para ami49, un factor de 2.
- **Hipótesis (no comprobada):** en nuestro modelo el disipador saca calor en proporción al área del die (`heatSinkH` por m²). En HotSpot el spreader (3 cm) y el disipador (6 cm) son fijos y mucho más grandes que cualquiera de los dos dies (1,3 mm y 7,7 mm), así que su resistencia térmica no escala igual con el tamaño del chip. Por eso un solo valor por m² no puede servir para dies de áreas tan distintas (el de ami49 es unas 26 veces más grande).

## 4. Conclusión

1. **Grilla:** N = 16 basta. De 16 a 64 el MAE cambia menos de 0,03 °C, y el cálculo pasa de 25 ms a 0,35 ms en ami33. N = 8 todavía sirve en ami33, pero en ami49 el MAE sube de 1,2 a 1,5 °C.
2. **Disipador:** el valor por defecto (4·10⁴) da errores de 10 a 18 °C y hay que cambiarlo. Calibrado para cada benchmark, el modelo rápido queda a 0,6 °C (ami33) y 1,2 °C (ami49) de HotSpot, y con N = 16 tarda 0,35 ms (ami33) y 0,24 ms (ami49), frente a unos 25 ms y 120–220 ms de HotSpot. La comparación de tiempos es aproximada, porque el tiempo de HotSpot incluye arrancar el programa y leer los archivos.
3. **Límite del modelo:** un único `heatSinkH` deja 4,7 °C de error medio entre benchmarks de tamaños muy distintos. Para el informe hay que decir que el disipador se calibra por benchmark, o mejorar el modelo del disipador (ver la sección 6).

## 5. Sin relleno (sensibilidad)

Con el `.flp` que exporta hoy el programa (solo bloques), HotSpot da una Tmax más alta: ami33 56,9 °C de media en vez de 54,2 °C, y ami49 74,0 °C en vez de 70,1 °C. La calibración cambia a 1,00·10⁵ (ami33) y 5,01·10⁴ (ami49), con MAE de 0,98 °C y 2,38 °C. El mejor valor común sería 6,31·10⁴, con 4,8 °C de MAE. O sea, el relleno cambia los números en 2 a 4 °C, pero no las conclusiones.

## 6. Decisiones para el equipo

- **Valor por defecto de `heatSinkH`** (Ismael, `Thermal.hpp`): con el procedimiento de SEMANA5 (mínimo error medio) sale **7,94·10⁴**, pero ese valor tiene 4,7 °C de error. Opciones: (a) usar 7,94·10⁴ y reportar el error; (b) calibrar por benchmark (1,26·10⁵ en ami33, 6,31·10⁴ en ami49) y pasarlo con `--heat-sink-h` (E12); (c) cambiar el modelo del disipador para que no dependa del área del die.
- **`siliconK`:** el comentario de `Thermal.hpp` dice que 100 W/(m·K) es el valor por defecto de HotSpot, pero `template.config` y `example.config` de HotSpot v7.0 usan `-k_chip 130.0`. Aquí se corrió HotSpot con `-k_chip 100` para comparar lo mismo; hay que decidir cuál usar en el informe.
- **Relleno en el `.flp`:** `exportHotSpotFLP` escribe solo los bloques, y su prueba (`tests/test_outputs.cpp`) exige exactamente una línea por bloque. Para que el programa genere el `.flp` con relleno hace falta una función nueva en `Exporters.hpp` (cambio de contrato). Mientras tanto, el relleno se generó con una herramienta aparte (sección 7).
- **Configuración de HotSpot:** se usó `example.config` (`-r_convec 0.1`). `template.config` usa `-r_convec 1.042`, que da temperaturas mucho más altas. El `heatSinkH` calibrado depende de esta elección.

## 7. Cómo repetirlo

1. Floorplans: `build/thermoplace benchmarks/mcnc/<bench>.block --out <dir> --sa --outline-weight 5 --seed <s>`, para cada benchmark y semilla.
2. Herramienta de J15 (C++, fuera del repo, enlazada con `src/`): lee el `.flp`, asigna la potencia de J7, escribe el `.flp` y el `.ptrace` con relleno, y guarda nuestras temperaturas por bloque para cada grilla y cada `heatSinkH`.
3. HotSpot, para cada floorplan: `./hotspot -c example.config -materials_file example.materials -model_type block -k_chip 100 -f <filled.flp> -p <filled.ptrace> -steady_file <out.steady>`.
4. Script de comparación (Python): toma las líneas de los bloques reales del `.steady` (en K, menos 273,15) y calcula MAE, error máximo y error en Tmax.

Cuando entre E12, `<bench>_best_temps.csv` usa la misma función (`blockTemperatures`), así que debería dar los mismos números. La herramienta y el script se pueden agregar al repo si el equipo está de acuerdo (por ejemplo, en una carpeta `tools/`).
