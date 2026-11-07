# HotSpot: instalación y primer ejemplo (J13)

HotSpot es el simulador térmico detallado con el que validamos nuestro modelo rápido en la semana 5 (J15). Esta guía dice cómo compilarlo y correr el ejemplo que trae, y cómo usarlo después con nuestros floorplans.

## Versión probada

- HotSpot **v7.0** (repositorio https://github.com/uvahotspot/HotSpot, commit `f18831e`, `git describe` = `v7.0-2-gf18831e`).
- PC de Jose: Ubuntu 26.04 LTS (x86_64), gcc 15.2.0, GNU make.
- Se compila sin SuperLU (`SUPERLU = 0` en el `Makefile`, el valor por defecto), así que solo necesita `gcc`, `make` y la librería matemática (`-lm`).

## 1. Instalar las herramientas

**Linux (Ubuntu):**

```bash
sudo apt install build-essential git
```

**Windows:** lo más fácil es WSL con Ubuntu. En PowerShell como administrador:

```bat
wsl --install
```

Reinicia, abre "Ubuntu" desde el menú de inicio y sigue con los pasos de Linux.

## 2. Descargar y compilar

Clónalo **fuera** de la carpeta de ThermoPlace (por ejemplo en tu carpeta personal), para no mezclarlo con el repositorio:

```bash
cd ~
git clone https://github.com/uvahotspot/HotSpot.git
cd HotSpot
make
```

`make` produce dos programas en `~/HotSpot`: `hotspot` (el simulador) y `hotfloorplan` (un floorplanner propio que no usamos).

## 3. Correr el ejemplo 1

```bash
cd ~/HotSpot/examples/example1
bash run.sh
```

`run.sh` simula el floorplan del procesador Alpha EV6 (`ev6.flp`) con la traza de potencia de `gcc.ptrace`, en **modo bloque** (`-model_type block`, una temperatura por bloque). Hace dos corridas:

1. Calcula el estado estacionario (`-steady_file outputs/gcc.steady`) y una traza transitoria (`-o outputs/gcc.ttrace`) partiendo de una temperatura uniforme.
2. Copia `gcc.steady` a `gcc.init` y repite la traza transitoria, ahora partiendo del estado estacionario.

Archivos que produce en `examples/example1/outputs/`:

| Archivo | Qué tiene |
|---|---|
| `gcc.steady` | Temperatura de equilibrio de cada bloque (y de las capas del encapsulado), en **kelvin**. Es lo que usamos en J15. |
| `gcc.ttrace` | Temperatura de cada bloque en cada paso de la traza de potencia (transitoria). No la usamos. |

Resultado en la PC de Jose: `make` compila sin errores y `run.sh` termina con `Simulation complete.` Así empieza `gcc.steady`, una línea por elemento con el nombre y la temperatura en K, separados por tabulador:

```
L2_left	324.35
L2	323.85
L2_right	324.69
Icache	330.84
Dcache	335.27
```

Formato de `gcc.steady` (132 líneas para los 30 bloques de `ev6.flp`):

| Líneas | Qué son |
|---|---|
| 1–30 | Los **bloques del chip**, con el mismo nombre que en el `.flp`. Son las que comparamos. |
| `iface_<bloque>` | Capa de material de interfaz térmica, debajo de cada bloque. |
| `hsp_<bloque>` | Spreader (difusor de calor), debajo de cada bloque. |
| `hsink_<bloque>` | Disipador, debajo de cada bloque. |
| `inode_0` … `inode_11` | Nodos de la periferia del spreader y del disipador. |

El bloque más caliente del ejemplo es `IntReg_1`: 343,01 K = 69,86 °C.

## 4. Usarlo con nuestros floorplans (para J15)

Nuestro programa ya genera los dos archivos que HotSpot necesita:

- `results/<bench>_best.flp`: floorplan en metros (`exportHotSpotFLP`, J5).
- `results/<bench>_best.ptrace`: potencia de cada bloque en watts (`exportPTrace`, J7), con `--thermal`.

Copia la configuración del ejemplo y corre HotSpot en modo bloque, solo estado estacionario (sin `-o`, que es más rápido):

```bash
cd ~/HotSpot
cp examples/example1/example.config examples/example1/example.materials .
./hotspot -c example.config -materials_file example.materials -model_type block \
    -f /ruta/a/ThermoPlace/results/ami33_best.flp \
    -p /ruta/a/ThermoPlace/results/ami33_best.ptrace \
    -steady_file ami33_best.steady
```

`ami33_best.steady` tiene el mismo formato que `gcc.steady` (sección 3): las primeras líneas son nuestros bloques (`bk1`, `bk10a`, …) en kelvin. Para comparar con nuestro `<bench>_best_temps.csv` (E12, en °C) hay que tomar solo esas líneas y restar 273,15.

**Prueba hecha (5 nov):** ami33 con `--sa --outline-weight 5 --seed 1` (15,41 % de espacio muerto, cabe) y potencia de J7 (semilla 1, 0,5–2 W/mm², 1,61 W en total). HotSpot corre sin errores y da 144 líneas (33 bloques + capas). Los bloques quedan entre **326,27 K (53,1 °C, `bk8a`)** y **329,16 K (56,0 °C, `bk7`)**. Nuestro modelo, con el `heatSinkH` por defecto, da una Tmax media de 73,3 °C para ami33 sin peso térmico (tabla de `docs/SEMANA5.md`). Esa diferencia es la que se calibra en J15.

## 5. Parámetros que importan para comparar

Valores del `example.config` del ejemplo 1, frente a los de nuestro `ThermalParams` (`include/thermoplace/Thermal.hpp`):

| Parámetro | HotSpot (`example.config`) | ThermoPlace | ¿Coinciden? |
|---|---|---|---|
| Ambiente | `-ambient 318.15` K = 45 °C | `ambientC = 45` | sí |
| Espesor del die | `-t_chip 0.00015` m | `dieThickness = 0.15e-3` | sí |
| Conductividad del silicio | `-k_chip 130.0` W/(m·K) | `siliconK = 100` | **no** |
| Disipador | `-r_convec 0.1` K/W, `-s_sink 0.06` m, `-t_sink 0.0069` m, `-s_spreader 0.03` m | `heatSinkH = 4e4` W/(m²·K), un solo número | otro modelo |
| Fugas (leakage) | `-leakage_used 0` | no modelado | sí |

- **k_chip:** el ejemplo 1 usa 130 W/(m·K) y nosotros 100. Para J15 hay que decidir cuál usar y poner el mismo en los dos (en HotSpot se cambia con `-k_chip 100`).
- **Disipador:** HotSpot modela el spreader, el disipador y la convección por separado. Nosotros lo resumimos en `heatSinkH`, que es justo lo que se calibra en J15.
- **Tamaño del die:** el disipador de HotSpot (6 × 6 cm) es mucho más grande que ami33 (~1,3 mm). Hay que tenerlo en cuenta al interpretar la calibración.
