# Semana 4 (4–10 nov): modelo térmico

**Meta:** calcular la temperatura de cada floorplan y verla. Al terminar, `build\thermoplace_tests.exe week4` debe dar **11/11**, el programa debe aceptar `--thermal` y HotSpot debe correr en al menos una PC (lo necesitamos para validar en la semana 5).

Empezamos con una semana de atraso. Por eso se recortó el plan (ver `docs/PLAN.md`): sale el contour O(n) y las semanas 6 y 7 se juntan.

| Persona | Tareas | Archivos | Prueba |
|---|---|---|---|
| **Ismael** | I11 modelo térmico (hecho, en este PR) | `src/Thermal.cpp` | `thermoplace_tests thermal` (6/6) |
| **Jose** | J7 potencia + `.ptrace` + CSV, J12 mapa de calor SVG, J13 instalar HotSpot | `src/Power.cpp`, `src/Exporters.cpp`, `docs/HOTSPOT.md` (nuevo) | `thermoplace_tests jose` |
| **Eimi** | E11 columna `max_temp_c`, opción `--thermal` | `src/Report.cpp`, `app/main.cpp` | `thermoplace_tests eimi` |

Contrato nuevo: `include/thermoplace/Thermal.hpp`. También cambiaron `Exporters.hpp` (`exportThermalSVG`) y `Report.hpp` (`maxTempC`).

## Cómo funciona el modelo (resumen)

- El chip se divide en una grilla de N × N celdas (por defecto 32). Cada celda recibe la potencia de los bloques que la cubren, según el área que cubren.
- El calor va hacia las 4 celdas vecinas (por el silicio) y hacia el ambiente (por el disipador). En equilibrio, lo que entra a una celda es igual a lo que sale.
- Se resuelve con Gauss-Seidel con sobre-relajación (SOR): se recorre la grilla actualizando cada celda hasta que nada cambie más de 10⁻⁶ °C.
- Valores por defecto (los de HotSpot): ambiente 45 °C, silicio de 100 W/(m·K) y 0,15 mm de espesor. `heatSinkH` = 4·10⁴ W/(m²·K) es un valor nuestro, que se ajusta contra HotSpot en la semana 5.

Prueba rápida (ami33 y ami49 después de SA, potencia de 0,5 a 2 W/mm², semilla 1):

| Benchmark | Potencia total | Tmax (grilla 16) | Tmax (grilla 32) | Tiempo (grilla 32) |
|---|---|---|---|---|
| ami33 | 1,6 W | 75,75 °C | 75,76 °C | 7 ms |
| ami49 | 50,5 W | 92,72 °C | 92,83 °C | 1 ms |

Una grilla de 16 ya da casi lo mismo que una de 32: dato útil para la semana 5, cuando la temperatura entre al SA y se calcule miles de veces.

## Jose

### J7. Potencia sintética, `.ptrace` y CSV (40 min) — ya estaba en `MANUAL_JOSE.md`

- `assignSyntheticPower`: `std::mt19937 rng(seed)`, densidad `u(minDensity, maxDensity)` y `power = densidad * area()`.
- `exportPTrace`: línea 1 con los nombres separados por tabulador, línea 2 con las potencias.
- `exportCSV`: encabezado `name,x,y,w,h,rotated,power` y una fila por bloque.
- **Rango que usamos en todo el proyecto:** de 0,5·10⁻⁶ a 2·10⁻⁶ W por µm² (0,5–2 W/mm²), semilla 1.

Prueba: `thermoplace_tests power_extra` y `exporters_extra`.

### J12. Mapa de calor (1 h) — `exportThermalSVG` al final de `src/Exporters.cpp`

Un `<rect class="cell">` por celda, con color de azul (más frío) a rojo (más caliente). La fila 0 va **abajo** (igual que en `exportSVG`). Agrega una barra de colores con la temperatura mínima y máxima, y el título. Devuelve `false` si `map.n == 0`.
Prueba: `thermoplace_tests thermal_svg`. Abre `results\test_thermal.svg` para verla.

### J13. Instalar HotSpot (1–2 h) — `docs/HOTSPOT.md`

HotSpot (https://github.com/uvahotspot/HotSpot) se compila con `make` en Linux. En Windows lo más fácil es **WSL** (Ubuntu): `wsl --install`, luego `sudo apt install build-essential git`, clonar y `make`. Corre el ejemplo `examples/example1` y anota en `docs/HOTSPOT.md` los pasos exactos y qué archivos produce (`.ttrace`, `.grid.steady`). Si no sale en tu PC, avisa ese mismo día para intentarlo en otra.

## Eimi

### E11a. Columna `max_temp_c` (15 min) — `src/Report.cpp`

`RunRecord` tiene un campo nuevo, `maxTempC`. Escríbelo al final de cada línea y agrega `,max_temp_c` al encabezado (el test `report` ya lo espera). Un `runs.csv` viejo tiene una columna menos: empieza uno nuevo.

### E11b. Opción `--thermal` (1 h) — `app/main.cpp`, cuando Jose una J7

- Opciones nuevas: `--thermal`, `--power-seed N` (1), `--grid N` (32).
- Con `--thermal`: después de cargar el benchmark, `assignSyntheticPower(d, powerSeed, 0.5e-6, 2e-6)`. Calcula `solveThermal` del floorplan inicial y del final, e imprime `Tmax` y `Tavg` de cada uno. Guarda `maxTempC` en el `runs.csv`, y escribe `<bench>_best.ptrace`, `<bench>_best.csv` y `<bench>_thermal.svg` (este último cuando Jose una J12).
- Ejemplo: `build\thermoplace.exe benchmarks\mcnc\ami33.block --sa --outline-weight 5 --thermal --seed 1`

## Cierre de la semana (martes 10)

- [ ] Los PR de los tres en `main`; `core`, `week2`, `week3` y `week4` en verde
- [ ] ami33 y ami49 con `--sa --thermal`, semillas 1 a 5, en un `runs.csv` nuevo
- [ ] HotSpot corre el ejemplo en al menos una PC, con los pasos en `docs/HOTSPOT.md`
