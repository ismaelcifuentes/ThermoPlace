# tools/ — validación contra HotSpot (J15)

Herramientas para repetir la validación de `docs/VALIDATION.md`. No forman parte del programa principal ni de las pruebas.

| Archivo | Qué hace |
|---|---|
| `j15_tool.cpp` | Lee un benchmark y un `.flp`, asigna la potencia de J7 (semilla 1, 0,5–2 W/mm²) y: (`filled`) escribe el `.flp` y el `.ptrace` para HotSpot, con el die rellenado con bloques de 0 W; (`sweep`) calcula nuestras temperaturas por bloque con grilla 32 para cada par (`heatSinkH`, `packageR`); (`grids`) las calcula con grillas 8, 16, 32 y 64 para un par dado. |
| `j15_compare.py` | Compara nuestras temperaturas con las de HotSpot. `calibrate` elige el par (`heatSinkH`, `packageR`) con menor error medio conjunto; `grids` da la tabla de errores y tiempos por grilla. |
| `run_j15.sh` | Corre todo: floorplans de SA (ami33 y ami49, semillas 1 a 5), relleno, HotSpot, barrido y comparación. |

## Requisitos

- Linux o WSL (HotSpot no corre en Windows nativo), con `g++` y `python3` (solo biblioteca estándar).
- ThermoPlace compilado: `./build.sh`.
- HotSpot compilado en `~/HotSpot` (ver `docs/HOTSPOT.md`). Si está en otra carpeta: `export HOTSPOT_DIR=/ruta/a/HotSpot`.

## Uso

Desde la raíz del repositorio:

```bash
./build.sh
bash tools/run_j15.sh calibrate         # unos 3 minutos
bash tools/run_j15.sh grids 1.4e5 0.20  # tabla por grilla con los valores elegidos
```

Todo se escribe en `build/j15/` (no se sube al repositorio): una carpeta `<bench>_s<semilla>` por floorplan, con el `.flp` del SA, el `.flp`/`.ptrace` con relleno, la salida de HotSpot (`j15_filled.steady`) y nuestros CSV.

## Por qué hace falta el relleno

HotSpot no acepta huecos en el floorplan, y nuestro modelo usa todo el outline como die (I13). `j15_tool` corta el die en franjas verticales en cada borde de bloque y convierte cada hueco de cada franja en un bloque de relleno de 0 W, hasta cubrir el 100 % del die. `exportHotSpotFLP` del programa principal escribe solo los bloques.
