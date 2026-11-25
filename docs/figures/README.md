# docs/figures/ — figuras del informe y la presentación

Todas corridas con la etiqueta `v1.0`. Cada figura está en PNG (diapositivas) y SVG (informe).

| Archivo | Figura (`docs/report/OUTLINE.md`) | Qué muestra | Autor |
|---|---|---|---|
| `fig3_ami33_floorplan_heatmap` | 3 | ami33 con peso térmico 0,25: el floorplan coloreado por densidad de potencia y su mapa de temperatura, lado a lado. El pico (80,1 °C) cae sobre el bloque más denso, bk1 | Jose |
| `fig3b_ami33_thermal_weight` | (extra, diapositiva 10) | ami33 con peso 0 y 0,25, con la misma escala de color: el pico baja de 84,2 a 80,1 °C; HotSpot confirma −3,7 °C de media en 5 semillas | Jose |
| `fig4_error_vs_grid` | 4 | Error contra HotSpot y tiempo de cálculo según la grilla: 16 × 16 tiene el mismo error que 64 × 64 (0,53 °C) y es 60–75 veces más rápida | Jose |
| `fig6_ami33_convergence` | 6 | Convergencia del SA en ami33 (semilla 1): el mejor costo baja de 2,42 a 0,34 en 180 temperaturas | Jose |
| `demo_ami33_run.png` / `.gif` | demo (plan B) | Salida del programa en ami33 con peso 0,25. El GIF muestra las líneas como aparecen en la corrida real | Jose |
| `pareto_e13.svg` | 5 | Frente de Pareto área vs. Tmax (E13) | Eimi |

Datos de las figuras 3 y 3b: semilla 5, potencia de 0,1 a 10 W/mm² (6,91 W), grilla 32. Se eligió la semilla 5 porque su bajada en HotSpot (−3,5 °C) es la más cercana a la media de las 5 semillas (−3,7 °C).

## Cómo regenerarlas

Linux o WSL, desde la raíz del repositorio, con Python 3 y `matplotlib` y `Pillow` (`sudo apt install python3-matplotlib python3-pil`). La figura 4 necesita HotSpot (`docs/HOTSPOT.md`).

```bash
git checkout v1.0 && ./build.sh
mkdir -p build/fig

# Figuras 3 y 3b: floorplans de peso 0 y 0,25 (semilla 5) y su mapa de celdas
g++ -std=c++17 -O2 -Iinclude docs/figures/scripts/thermal_grid.cpp src/Parser.cpp \
    src/Power.cpp src/Thermal.cpp src/Metrics.cpp -o build/fig/thermal_grid
for w in 0 0.25; do
  mkdir -p build/fig/run_w$w
  ./build/thermoplace benchmarks/mcnc/ami33.block --out build/fig/run_w$w --sa --outline-weight 5 \
      --thermal --thermal-weight $w --power-min 0.1 --power-max 10 --seed 5 \
      --runs-csv build/fig/run_w$w/runs.csv
  ./build/fig/thermal_grid benchmarks/mcnc/ami33.block build/fig/run_w$w/ami33_best.flp \
      build/fig/ami33_w${w}_s5 0.1 10
done

# Figura 4: corridas de J15 (tools/README.md)
J15_OUT=build/fig/j15 bash tools/run_j15.sh validate 1.4e5 0.20

# Figura 6: traza del SA de ami33, semilla 1 (la misma corrida de J15)
cp build/fig/j15/ami33_s1/ami33_trace.csv build/fig/

python3 docs/figures/scripts/make_figures.py build/fig docs/figures

# Captura de la demo
./build/thermoplace benchmarks/mcnc/ami33.block --out results --sa --outline-weight 5 --thermal \
    --thermal-weight 0.25 --power-min 0.1 --power-max 10 --seed 5 > build/fig/demo.txt
python3 docs/figures/scripts/terminal_capture.py build/fig/demo.txt \
    "./build/thermoplace benchmarks/mcnc/ami33.block --out results --sa --outline-weight 5 --thermal --thermal-weight 0.25 --power-min 0.1 --power-max 10 --seed 5" \
    docs/figures/demo_ami33_run
```

`scripts/thermal_grid.cpp` usa el mismo modelo térmico que el programa (grilla 32, valores por defecto de `v1.0`) y solo exporta la temperatura de cada celda; la Tmax que da es la misma que imprime el programa.
