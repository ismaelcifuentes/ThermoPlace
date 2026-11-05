# Semana 5 (11–17 nov): temperatura dentro del SA y validación con HotSpot

**Meta:** que el SA pueda bajar la temperatura máxima y medir cuánto cuesta en área (frente de Pareto), y comparar nuestro modelo rápido con HotSpot. Estos son los dos resultados centrales del informe.

Antes de empezar, la semana 4 debe estar en `main` (J7, J12, J13, E11).

| Persona | Tareas | Archivos | Prueba |
|---|---|---|---|
| **Ismael** | I12 evaluador con arranque en caliente, I13 el die es el outline, I14 temperatura por bloque (hecho) | `src/Thermal.cpp` | `thermoplace_tests thermal_sa` (3/3) |
| **Jose** | J14 costo con término térmico, J15 validación contra HotSpot | `src/Cost.cpp`, `docs/VALIDATION.md` (nuevo) | `thermoplace_tests cost_thermal` |
| **Eimi** | E12 opciones `--thermal-weight`, `--heat-sink-h` y CSV de temperaturas; E13 frente de Pareto | `app/main.cpp`, `docs/RESULTS.md` | correr el programa |

## Ismael (hecho)

- **I12 `ThermalEvaluator`:** calcula la temperatura máxima empezando desde el mapa anterior (arranque en caliente). Dos floorplans seguidos del SA casi no cambian, así que se necesitan pocas iteraciones. Se usa dentro del lambda de costo.
- **I13 `dieIsOutline`:** si el benchmark tiene outline, la grilla cubre todo el outline, no solo los bloques. Sin esto, el SA "enfriaba" el chip simplemente separando los bloques, porque un chip más grande tiene más área de disipador. Ahora el SA solo puede bajar la temperatura reacomodando los bloques dentro del mismo die.
- **I14 `blockTemperatures`:** la temperatura media de cada bloque. HotSpot en modo bloque da un valor por bloque, así que esto es lo que se compara (J15).

Prueba de concepto (3 semillas, `--outline-weight 5`, potencia de 0,5–2 W/mm² con semilla 1, grilla 16, tolerancia 10⁻³; Tmax final medida con grilla 32):

| Benchmark | Peso térmico | Tmax media | Espacio muerto medio | Cabe | Tiempo medio |
|---|---|---|---|---|---|
| ami33 | 0 | 73,3 °C | 15,2 % | 3/3 | 0,2 s |
| ami33 | 0,5 | 71,2 °C | 24,2 % | 3/3 | 7,3 s |
| ami33 | 2 | 65,2 °C | 43,8 % | 0/3 | 6,7 s |
| ami49 | 0 | 92,2 °C | 12,4 % | 1/3 | 0,6 s |
| ami49 | 0,5 | 86,6 °C | 15,7 % | 0/3 | 5,9 s |
| ami49 | 2 | 83,9 °C | 23,5 % | 0/3 | 6,2 s |

Bajar la temperatura cuesta área, que es justo lo que muestra un frente de Pareto. Con peso 2 el chip ya no cabe en el outline, así que el barrido útil está entre 0 y 1.

## Jose

### J14. Costo con término térmico (15 min) — `src/Cost.cpp`

Nueva versión de `computeCost` con un parámetro más, `tempRise` (Tmax − ambiente): devuelve el costo de siempre **más** `thermal * tempRise / norm.tempRise`. Si `norm.tempRise <= 0`, el término térmico vale 0. Reutiliza la versión de 4 parámetros para no repetir la fórmula.
Prueba: `thermoplace_tests cost_thermal`.

### J15. Validación contra HotSpot (1–2 días) — `docs/VALIDATION.md`

Es la pregunta de investigación: **¿qué tan simple puede ser el modelo sin perder precisión?**

1. Con el programa de Eimi (`--thermal`), genera `.flp`, `.ptrace` y el CSV de temperaturas por bloque de los mejores floorplans de ami33 y ami49 (semillas 1 a 5).
2. Corre HotSpot en **modo bloque** con esos archivos y guarda la temperatura de cada bloque.
3. Compara con nuestras temperaturas por bloque para grillas de 8, 16, 32 y 64: error medio absoluto, error máximo y error en Tmax, más el tiempo de cálculo de cada grilla.
4. **Calibración:** prueba varios `--heat-sink-h` y elige el que deja el menor error medio contra HotSpot. Ese valor queda como el nuevo valor por defecto (avísale a Ismael para cambiarlo en `Thermal.hpp`).
5. En `docs/VALIDATION.md`: los pasos, la versión y configuración de HotSpot, una tabla de errores por grilla y una conclusión de 2–3 frases.

## Eimi

### E12. Opciones nuevas (1 h) — `app/main.cpp`, cuando Jose una J14

- `--thermal-weight W` (requiere `--thermal`): crea un `ThermalEvaluator` con grilla 16 y tolerancia 10⁻³. Usa `norm.tempRise = Tmax_inicial − ambiente` y, en el lambda de costo, la versión de `computeCost` con `tempRise`.
- `--heat-sink-h H`: cambia `ThermalParams::heatSinkH` (para la calibración de Jose).
- Con `--thermal`, escribe también `<bench>_best_temps.csv` con `name,temp_c` usando `blockTemperatures` (lo usa Jose en J15).

### E13. Frente de Pareto área vs. Tmax (1 día) — `docs/RESULTS.md`

- Pesos térmicos 0, 0,25, 0,5, 0,75 y 1; semillas 1 a 5; ami33 y ami49; `--sa --outline-weight 5 --thermal`.
- Tabla por peso: Tmax media, espacio muerto medio, HPWL medio, cuántas caben y el tiempo.
- Una gráfica de espacio muerto (eje x) contra Tmax (eje y), un punto por peso. Sirve hacerla en Excel o Google Sheets desde `runs.csv`.

## Cierre de la semana (martes 17)

- [ ] J14 y E12 en `main`, `week5` en verde
- [ ] Tabla y gráfica del frente de Pareto en `docs/RESULTS.md`
- [ ] Tabla de errores contra HotSpot y `heatSinkH` calibrado en `docs/VALIDATION.md`
