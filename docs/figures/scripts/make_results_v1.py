"""Builds docs/RESULTS.md and the two figures from the v1.0 runs (E10, E13, E14, E6).

Usage: python make_results_v1.py <v1 csv dir> <old csv dir> <RESULTS.md in> <RESULTS.md out> <figures dir>

<v1 csv dir>  runs.csv, pareto.csv, sensitivity.csv, gsrc.csv written by run_v1.bat (tag v1.0)
<old csv dir> the same four files from before v1.0 (quadratic outline penalty), for the comparison
Jose's sections (HPWL definition and the J17 bullet) and the published-results notes are
copied unchanged from <RESULTS.md in>.
"""
import csv
import math
import sys
from collections import defaultdict
from statistics import mean

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

V1, OLD, MD_IN, MD_OUT, FIG = sys.argv[1:6]

BLOCK_AREA = {"ami33": 1156449, "ami49": 35445424}  # um^2, printed by thermoplace
OUTLINE_FREE = {"ami33": 27.6, "ami49": 13.4}       # % free space of the benchmark outline
POWER_E13 = {"ami33": 1.61, "ami49": 50.53}         # W, J7 power seed 1, 0.5-2 W/mm2
POWER_E14 = {"ami33": 6.91, "ami49": 220.05}        # W, 0.1-10 W/mm2
AMB, PKG_R = 45.0, 0.20
NBLOCKS = {"n100": 100, "n200": 200, "n300": 300}


def num(x, d):
    return f"{x:,.{d}f}".replace(",", " ").replace(".", ",")


def load(path):
    # Keep the first run of each seed: the old runs.csv also has two later test runs.
    rows, seen = defaultdict(list), set()
    for r in csv.DictReader(open(path, newline="")):
        key = (r["benchmark"], r["method"], r["outline_weight"])
        if key + (r["seed"],) in seen:
            continue
        seen.add(key + (r["seed"],))
        rows[key].append(r)
    return rows


def col(rs, c):
    return [float(r[c]) for r in rs]


def fits(rs):
    return f"{sum(int(r['fits_outline']) for r in rs)}/{len(rs)}"


new = {k: load(f"{V1}/{k}.csv") for k in ("runs", "pareto", "sensitivity", "gsrc")}
old = {k: load(f"{OLD}/{k}.csv") for k in ("runs", "pareto", "sensitivity", "gsrc")}
for k in new:
    n = sum(len(v) for v in new[k].values())
    expected = {"runs": 30, "pareto": 50, "sensitivity": 30, "gsrc": 20}[k]
    if n != expected:
        sys.exit(f"{k}.csv has {n} runs, expected {expected}")

md_in = open(MD_IN, encoding="utf-8").read()


def between(text, start, end):
    i = text.index(start)
    j = text.index(end, i)
    return text[i:j]


published = between(md_in, "## Resultados publicados", "## Definición de HPWL")
hpwl_sec = between(md_in, "## Definición de HPWL", "**Figuras finales**")
j17 = between(md_in, "- **Validación en HotSpot (J17", "## Escalabilidad en GSRC")

L = []
w = L.append

E10 = [
    ("random", "0", "Búsqueda aleatoria, 20 000 iteraciones"),
    ("sa", "5", "SA, `--outline-weight 5`"),
    ("sa_c0.98_m2000", "5", "SA largo, `--outline-weight 5 --cooling 0.98 --moves-per-temp 2000`"),
]


def e10_table(src, bench):
    w("| Método | Corridas | Espacio muerto medio | Mejor espacio muerto | HPWL medio | Mejor HPWL | Área media (mm²) | Cabe | Tiempo medio (s) |")
    w("|---|---|---|---|---|---|---|---|---|")
    for m, ow, label in E10:
        rs = src[(bench, m, ow)]
        ds, hp = col(rs, "dead_space_pct"), col(rs, "hpwl")
        ar = [a / 1e6 for a in col(rs, "chip_area")]
        w(f"| {label} | {len(rs)} | {num(mean(ds), 2)} % | {num(min(ds), 2)} % | {num(mean(hp), 0)} | "
          f"{num(min(hp), 0)} | {num(mean(ar), 3)} | {fits(rs)} | {num(mean(col(rs, 'seconds')), 2)} |")
    w("")


# ---------------------------------------------------------------- header
w("# Resultados (corrido con v1.0)")
w("")
w("Todas las tablas de este archivo, salvo la última sección, se corrieron con la etiqueta **`v1.0`** "
  "(commit `3a1ca55`): penalización de outline **lineal** (J18, PR #32), disipador calibrado "
  "(h = 1,4e5 W/(m²·K), R = 0,20 K/W) e I16. Semillas 1 a 5, `--alpha 0.5`. Máquina: PC de Eimi, "
  "Windows 11, g++ de MinGW con -O2 (build.bat). Los datos crudos están en `results/v1/` (no se suben), "
  "generados con `run_v1.bat`; este archivo se generó a partir de ellos. Los resultados son deterministas "
  "(la misma semilla da el mismo floorplan en cualquier PC); solo los tiempos dependen de la máquina.")
w("")
w("Los resultados con la penalización cuadrática de antes de `v1.0` quedan al final, en "
  "[Comparación: penalización cuadrática](#comparación-penalización-cuadrática-antes-de-v10).")
w("")
w("Cómo leer las tablas:")
w("")
w("- **Espacio muerto** = 1 − (área de los bloques / área del chip).")
w("- **Mejor** es el mínimo de esa columna entre las 5 semillas; el mejor espacio muerto y el mejor HPWL pueden venir de semillas distintas.")
w("- **Cabe** cuenta las semillas cuyo resultado entra en el outline fijo del benchmark.")
w("- La búsqueda aleatoria solo minimiza área y no sabe del outline; en los CSV aparece con `outline_weight` 0.")
w("")

# ---------------------------------------------------------------- E10
w("## Área y cables en ami33 y ami49 (E10, corrido con v1.0)")
w("")
for b in ("ami33", "ami49"):
    w(f"### {b}")
    w("")
    e10_table(new["runs"], b)

# Ablation table
w("### Penalización cuadrática contra lineal (J18)")
w("")
w("Hasta `v1.0` la penalización de outline era (exW/W)² + (exH/H)²: salirse un 2 % costaba 0,0004, "
  "así que cerca del borde el SA casi no tenía nada que lo empujara adentro. J18 la cambió a "
  "exW/W + exH/H (salirse un 2 % cuesta 0,02). Mismas semillas y mismos comandos; solo cambia la penalización:")
w("")
w("| Corrida | Espacio muerto medio (cuadrática → lineal) | HPWL medio (cuadrática → lineal) | Cabe (cuadrática → lineal) |")
w("|---|---|---|---|")
abl = [("runs", "ami33", "sa", "5", "ami33, SA"),
       ("runs", "ami33", "sa_c0.98_m2000", "5", "ami33, SA largo"),
       ("runs", "ami49", "sa", "5", "ami49, SA"),
       ("runs", "ami49", "sa_c0.98_m2000", "5", "ami49, SA largo"),
       ("gsrc", "n100", "sa", "5", "n100, outline con 15 % libre")]
for f, b, m, ow, label in abl:
    o, n = old[f][(b, m, ow)], new[f][(b, m, ow)]
    w(f"| {label} | {num(mean(col(o, 'dead_space_pct')), 2)} % → {num(mean(col(n, 'dead_space_pct')), 2)} % | "
      f"{num(mean(col(o, 'hpwl')), 0)} → {num(mean(col(n, 'hpwl')), 0)} | {fits(o)} → {fits(n)} |")
w("")
a33o, a33n = old["runs"][("ami33", "sa", "5")], new["runs"][("ami33", "sa", "5")]
a49n = new["runs"][("ami49", "sa_c0.98_m2000", "5")]
w(f"- Con la lineal **todas las corridas de SA caben** en el outline, también ami49, cuyo outline deja solo "
  f"{num(OUTLINE_FREE['ami49'], 1)} % de espacio libre (antes cabían 3/5 con SA y 2/5 con SA largo).")
w(f"- En ami33 baja a la vez el espacio muerto ({num(mean(col(a33o, 'dead_space_pct')), 1)} % → "
  f"{num(mean(col(a33n, 'dead_space_pct')), 1)} % con SA) y el HPWL "
  f"({num(100 * (1 - mean(col(a33n, 'hpwl')) / mean(col(a33o, 'hpwl'))), 1)} % menos): el outline empuja "
  "hacia un chip más compacto, que además acorta los cables.")
a49o_sa, a49n_sa = old["runs"][("ami49", "sa", "5")], new["runs"][("ami49", "sa", "5")]
w(f"- En ami49 con SA corto el HPWL sube un "
  f"{num(100 * (mean(col(a49n_sa, 'hpwl')) / mean(col(a49o_sa, 'hpwl')) - 1), 1)} %: es lo que cuesta que las 5 semillas "
  "quepan, cuando antes 2 de 5 se salían.")
w("")

# Best floorplans
def best_seed(rs):
    return min(rs, key=lambda r: float(r["dead_space_pct"]))


bests = {b: best_seed(new["runs"][(b, "sa_c0.98_m2000", "5")]) for b in ("ami33", "ami49")}
b33, b49 = bests["ami33"], bests["ami49"]
pub_head, pub_notes = published.split("Notas sobre esta tabla:")
w(pub_head.rstrip())
w("")
w(f"**Nuestro mejor resultado frente a [1].** Con `v1.0`, el mejor SA largo da "
  f"{num(float(b33['chip_area']) / 1e6, 3)} mm² en ami33 (semilla {b33['seed']}, "
  f"{num(float(b33['dead_space_pct']), 2)} % de espacio muerto) y {num(float(b49['chip_area']) / 1e6, 2)} mm² en ami49 "
  f"(semilla {b49['seed']}, {num(float(b49['dead_space_pct']), 2)} %), dentro del outline y optimizando también el HPWL. "
  "[1] reporta 1,27 y 36,80 mm² optimizando solo área y sin outline. En ami33 quedamos por debajo de [1]; en ami49, "
  f"{num(100 * (float(b49['chip_area']) / 1e6 / 36.80 - 1), 1)} % por encima. Son mejores semillas, no medias, y [1] "
  "no da la dispersión entre corridas, así que no es una comparación de igual a igual.")
w("")
w(("Notas sobre esta tabla:" + pub_notes).rstrip())
w("")
w(hpwl_sec.rstrip())
w("")
w("**Figuras finales** (se generan en `results/`, que no se sube; corrido con v1.0):")
w("")
w("| Benchmark | Comando | Espacio muerto | Cabe |")
w("|---|---|---|---|")
for b in ("ami33", "ami49"):
    r = bests[b]
    w(f"| {b} | `--sa --outline-weight 5 --cooling 0.98 --moves-per-temp 2000 --seed {r['seed']}` | "
      f"{num(float(r['dead_space_pct']), 2)} % | {'sí' if r['fits_outline'] == '1' else 'no'} |")
w("")
w("Cada corrida deja `<bench>_best.svg` (bloques, contorno del chip en negro y outline fijo en rojo) y "
  "`<bench>_convergence.svg` (costo actual y mejor costo por temperatura).")
w("")

# ---------------------------------------------------------------- E13
WEIGHTS = [("sa", "0"), ("sa_t0.25", "0,25"), ("sa_t0.5", "0,5"), ("sa_t0.75", "0,75"), ("sa_t1", "1")]


def thermal_table(src, bench, methods, with_bench=False):
    for m, label in methods:
        rs = src[(bench, m, "5")]
        t = col(rs, "max_temp_c")
        lead = f"| {bench} | {label}" if with_bench else f"| {label}"
        w(f"{lead} | {num(mean(t), 2)} | {num(min(t), 2)} | {num(mean(col(rs, 'dead_space_pct')), 2)} % | "
          f"{num(mean(col(rs, 'hpwl')), 0)} | {fits(rs)} | {num(mean(col(rs, 'seconds')), 2)} |")


P = new["pareto"]


def tm(src, b, m):
    return mean(col(src[(b, m, "5")], "max_temp_c"))


def ds(src, b, m):
    return mean(col(src[(b, m, "5")], "dead_space_pct"))


w("## Frente de Pareto: área contra Tmax (E13, corrido con v1.0)")
w("")
w("Corridas: `--sa --outline-weight 5 --thermal --thermal-weight W --heat-sink-h 1.4e5 --package-r 0.20`, "
  "pesos 0, 0,25, 0,5, 0,75 y 1, semillas 1 a 5, potencia de J7 (semilla 1, 0,5 a 2 W/mm²). Dentro del SA la "
  "temperatura se calcula con grilla 16; la Tmax de la tabla es la del mejor floorplan con grilla 32. "
  "Datos: `results/v1/pareto.csv`.")
w("")
for b in ("ami33", "ami49"):
    w(f"### {b}")
    w("")
    w("| Peso térmico | Tmax media (°C) | Mejor Tmax (°C) | Espacio muerto medio | HPWL medio | Cabe | Tiempo medio (s) |")
    w("|---|---|---|---|---|---|---|")
    thermal_table(P, b, WEIGHTS)
    w("")
w("![Frente de Pareto de ami33 y ami49 con v1.0](figures/pareto_e13.svg)")
w("")
a0, a25, a1 = tm(P, "ami33", "sa"), tm(P, "ami33", "sa_t0.25"), tm(P, "ami33", "sa_t1")
b0 = tm(P, "ami49", "sa")
b_all = [tm(P, "ami49", m) for m, _ in WEIGHTS]
p33, p49 = AMB + PKG_R * POWER_E13["ami33"], AMB + PKG_R * POWER_E13["ami49"]
w("Qué muestran los datos:")
w("")
w(f"- **ami33:** el peso 0,25 baja la Tmax media {num(a0 - a25, 2)} °C ({num(a0, 2)} a {num(a25, 2)} °C) con "
  f"{num(ds(P, 'ami33', 'sa_t0.25') - ds(P, 'ami33', 'sa'), 1)} puntos más de espacio muerto. El peso 1 baja "
  f"{num(a0 - a1, 2)} °C con {num(ds(P, 'ami33', 'sa_t1') - ds(P, 'ami33', 'sa'), 1)} puntos más. "
  "Con la penalización lineal los 25 floorplans con peso caben en el outline (antes, con peso 1, cabía 1 de 5).")
w(f"- **ami49:** la Tmax media se queda entre {num(min(b_all), 2)} y {num(max(b_all), 2)} °C para todos los pesos, "
  "una diferencia del tamaño de la dispersión entre semillas (puntos tenues de la figura). Con los pesos 0,25 y 0,5 "
  "queda incluso un poco por encima de la del peso 0. El espacio muerto tampoco sube "
  f"({num(min(ds(P, 'ami49', m) for m, _ in WEIGHTS), 1)} a {num(max(ds(P, 'ami49', m) for m, _ in WEIGHTS), 1)} %).")
w(f"- **Por qué ami49 no se mueve.** Su outline deja solo {num(OUTLINE_FREE['ami49'], 1)} % de espacio libre "
  f"(ami33: {num(OUTLINE_FREE['ami33'], 1)} %). Con la penalización lineal el SA ya no se puede salir del outline "
  "para separar los bloques calientes, así que no tiene dónde ganar grados. En ami33 el peso térmico sí usa el "
  "espacio libre: el espacio muerto sube hasta unos 22 %, todavía dentro del outline.")
w(f"- **Parte que depende de la colocación.** Con el paquete de dos partes (I15), toda la potencia pasa por "
  f"`packageR`, que sube el nodo del paquete a 45 + 0,20 · P: {num(p33, 2)} °C en ami33 y {num(p49, 2)} °C en ami49. "
  f"Lo que la colocación cambia es Tmax − T_paquete: en ami33 baja de {num(a0 - p33, 2)} a {num(a1 - p33, 2)} K con el "
  f"peso 1 ({num(100 * (a0 - a1) / (a0 - p33), 0)} %); en ami49 de {num(b0 - p49, 2)} a {num(min(b_all) - p49, 2)} K "
  f"en el mejor caso ({num(100 * (b0 - min(b_all)) / (b0 - p49), 0)} %).")
w("- **Límites.** Una sola asignación de potencia (semilla 1) y un solo par de valores del disipador.")
w("")

# ---------------------------------------------------------------- E14
S = new["sensitivity"]
SW = [("sa_p0.1-10", "0"), ("sa_t0.25_p0.1-10", "0,25"), ("sa_t0.5_p0.1-10", "0,5")]
w("## Sensibilidad a la potencia (E14, corrido con v1.0)")
w("")
w("Mismas corridas que E13 pero con densidades de potencia de **0,1 a 10 W/mm²** (semilla de potencia 1), pesos "
  "térmicos 0, 0,25 y 0,5, semillas 1 a 5: `--sa --outline-weight 5 --thermal --thermal-weight W --power-min 0.1 "
  f"--power-max 10 --heat-sink-h 1.4e5 --package-r 0.20`. La potencia total sube a {num(POWER_E14['ami33'], 2)} W en "
  f"ami33 y {num(POWER_E14['ami49'], 2)} W en ami49 (con 0,5 a 2 W/mm² era {num(POWER_E13['ami33'], 2)} y "
  f"{num(POWER_E13['ami49'], 1)} W). Datos: `results/v1/sensitivity.csv`. Con este rango de potencia el modelo "
  "subestima la Tmax de ami49 en unos 5 °C frente a HotSpot (J17, `docs/VALIDATION.md`), así que las temperaturas de "
  "ami49 sirven para comparar pesos entre sí, no como valores absolutos.")
w("")
w("| Benchmark | Peso térmico | Tmax media (°C) | Mejor Tmax (°C) | Espacio muerto medio | HPWL medio | Cabe | Tiempo medio (s) |")
w("|---|---|---|---|---|---|---|---|")
thermal_table(S, "ami33", SW, True)
thermal_table(S, "ami49", SW, True)
w("")
s0, s25, s5 = (tm(S, "ami33", m) for m, _ in SW)
t0, t25, t5 = (tm(S, "ami49", m) for m, _ in SW)
q33, q49 = AMB + PKG_R * POWER_E14["ami33"], AMB + PKG_R * POWER_E14["ami49"]
w("Qué cambia frente a E13:")
w("")
w(f"- **ami33:** con el peso 0,25 la Tmax media baja {num(s0 - s25, 2)} °C ({num(s0, 2)} a {num(s25, 2)} °C) con "
  f"{num(ds(S, 'ami33', 'sa_t0.25_p0.1-10') - ds(S, 'ami33', 'sa_p0.1-10'), 1)} puntos más de espacio muerto, y los 5 "
  f"floorplans caben. En E13 el mismo peso bajaba {num(a0 - a25, 2)} °C. El peso 0,5 casi no agrega: "
  f"{num(s0 - s5, 2)} °C en total, con {num(ds(S, 'ami33', 'sa_t0.5_p0.1-10'), 1)} % de espacio muerto.")
w(f"- **ami49:** el peso térmico no baja la Tmax: {num(t0, 2)} °C con peso 0, {num(t25, 2)} con 0,25 y {num(t5, 2)} "
  f"con 0,5, y con peso solo caben {fits(S[('ami49', 'sa_t0.25_p0.1-10', '5')])} floorplans. Es el mismo límite que en "
  "E13: el outline casi no deja espacio libre.")
w(f"- **Parte que depende de la colocación.** El nodo del paquete queda en {num(q33, 2)} °C (ami33) y {num(q49, 2)} °C "
  f"(ami49). En ami33 el peso 0,25 baja Tmax − T_paquete de {num(s0 - q33, 2)} a {num(s25 - q33, 2)} K "
  f"({num(100 * (s0 - s25) / (s0 - q33), 0)} %); en E13, {num(100 * (a0 - a25) / (a0 - p33), 0)} %. En proporción es "
  f"parecido, pero sobre una subida de {num(s0 - q33, 1)} K en vez de {num(a0 - p33, 1)} K, así que da casi "
  f"{num((s0 - s25) / (a0 - a25), 0)} veces más grados con menos área.")
w(j17.rstrip())
w("")

# ---------------------------------------------------------------- E6
G = new["gsrc"]
w("## Escalabilidad en GSRC (E6, corrido con v1.0)")
w("")
w("SA con los valores por defecto (enfriamiento 0,95, 10·n movidas por temperatura), `--alpha 0.5`, semillas 1 a 5. "
  "Los archivos de GSRC no traen outline: las tres primeras filas van sin outline, y la última con "
  "`--outline-weight 5 --whitespace 0.15` (outline cuadrado de (1,15 · área de bloques)^½ de lado, 15 % de espacio "
  "libre, como en la literatura de outline fijo [1]). Datos: `results/v1/gsrc.csv`.")
w("")
w("| Benchmark | Bloques | Outline | Espacio muerto medio | Mejor espacio muerto | HPWL medio | Cabe | Tiempo medio (s) |")
w("|---|---|---|---|---|---|---|---|")
times = {}
for b in ("n100", "n200", "n300"):
    rs = G[(b, "sa", "0")]
    times[b] = mean(col(rs, "seconds"))
    w(f"| {b} | {NBLOCKS[b]} | no | {num(mean(col(rs, 'dead_space_pct')), 2)} % | {num(min(col(rs, 'dead_space_pct')), 2)} % | "
      f"{num(mean(col(rs, 'hpwl')), 0)} | — | {num(times[b], 2)} |")
rs = G[("n100", "sa", "5")]
w(f"| n100 | 100 | 15 % libre | {num(mean(col(rs, 'dead_space_pct')), 2)} % | {num(min(col(rs, 'dead_space_pct')), 2)} % | "
  f"{num(mean(col(rs, 'hpwl')), 0)} | {fits(rs)} | {num(mean(col(rs, 'seconds')), 2)} |")
w("")
w("![Tiempo de SA contra número de bloques en GSRC](figures/gsrc_time.svg)")
w("")
expo = math.log(times["n300"] / times["n100"]) / math.log(3)
o15 = old["gsrc"][("n100", "sa", "5")]
w(f"- **Tiempo.** De n100 a n300 el tiempo crece {num(times['n300'] / times['n100'], 1)} veces, como n elevado a "
  f"{num(expo, 1)}. Era de esperar algo cercano a n³: el SA hace 10·n movidas por temperatura y cada empaquetado cuesta "
  "O(n²) (cada uno de los n bloques consulta un contorno de hasta n segmentos), porque el contorno O(n) de [2] quedó "
  f"fuera del alcance (`docs/PLAN.md`). n300 tarda {num(times['n300'], 0)} s por corrida.")
w("- **Calidad.** Sin outline el espacio muerto medio sube poco con el tamaño (" +
  ", ".join(num(mean(col(G[(b, 'sa', '0')], 'dead_space_pct')), 1) + " %" for b in ("n100", "n200", "n300")) +
  "), sin cambiar el número de temperaturas. Estas tres filas dan lo mismo que antes de `v1.0` (sin outline la "
  "penalización vale 0); solo cambian los tiempos.")
w(f"- **Outline fijo de 15 %.** Con la penalización lineal n100 cabe en {fits(rs)} semillas, con "
  f"{num(mean(col(rs, 'dead_space_pct')), 1)} % de espacio muerto medio. Con la cuadrática cabía en {fits(o15)}, con "
  f"{num(mean(col(o15, 'dead_space_pct')), 1)} %: salirse 10 unidades de 454 en ancho y en alto sumaba unos 0,005 al "
  "costo (con peso 5), frente a unos 0,3 de área y HPWL, y el SA no tenía nada que lo empujara adentro. Este caso fue el que llevó "
  "a J18.")
w("")
w("[1] S. N. Adya e I. L. Markov, \"Fixed-outline floorplanning: enabling hierarchical design\", *IEEE Trans. VLSI Syst.*, vol. 11, no. 6, pp. 1120–1135, dic. 2003.")
w("")
w("[2] Y.-C. Chang, Y.-W. Chang, G.-M. Wu y S.-W. Wu, \"B*-Trees: A New Representation for Non-Slicing Floorplans\", DAC 2000, pp. 458–463.")
w("")

# ---------------------------------------------------------------- old tables
w("## Comparación: penalización cuadrática (antes de v1.0)")
w("")
w("Mismas corridas con la penalización de outline cuadrática, antes de J18. Se dejan para comparar; los números del "
  "informe salen de las secciones de arriba. Tiempos en la PC de Eimi.")
w("")
w("### E10 (commit posterior al PR #11)")
w("")
for b in ("ami33", "ami49"):
    w(f"**{b}**")
    w("")
    e10_table(old["runs"], b)
w("### E13 (commit `1d76c82`, anterior a I16)")
w("")
for b in ("ami33", "ami49"):
    w(f"**{b}**")
    w("")
    w("| Peso térmico | Tmax media (°C) | Mejor Tmax (°C) | Espacio muerto medio | HPWL medio | Cabe | Tiempo medio (s) |")
    w("|---|---|---|---|---|---|---|")
    thermal_table(old["pareto"], b, WEIGHTS)
    w("")
w("### E14 (commit `590fd9f`, anterior a I16)")
w("")
w("| Benchmark | Peso térmico | Tmax media (°C) | Mejor Tmax (°C) | Espacio muerto medio | HPWL medio | Cabe | Tiempo medio (s) |")
w("|---|---|---|---|---|---|---|---|")
thermal_table(old["sensitivity"], "ami33", SW, True)
thermal_table(old["sensitivity"], "ami49", SW, True)
w("")
w("Con la cuadrática el peso 0,25 bajaba la Tmax de ami49 1,93 °C, pero solo cabían 2 de 5 floorplans: los que se "
  "salían tenían un die más grande, con más espacio para separar los bloques calientes (`docs/VALIDATION.md`, sección 8).")
w("")
w("### E6, n100 con 15 % de espacio libre (commit `e4538cb`)")
w("")
w("| Benchmark | Outline | Espacio muerto medio | Mejor espacio muerto | HPWL medio | Cabe | Tiempo medio (s) |")
w("|---|---|---|---|---|---|---|")
w(f"| n100 | 15 % libre | {num(mean(col(o15, 'dead_space_pct')), 2)} % | {num(min(col(o15, 'dead_space_pct')), 2)} % | "
  f"{num(mean(col(o15, 'hpwl')), 0)} | {fits(o15)} | {num(mean(col(o15, 'seconds')), 2)} |")
w("")

open(MD_OUT, "w", encoding="utf-8", newline="\n").write("\n".join(L))

# ---------------------------------------------------------------- figures
INK, INK2, SERIES, GRID, SURF = "#0b0b0b", "#52514e", "#2a78d6", "#e4e3df", "#fcfcfb"
plt.rcParams.update({"font.size": 10, "axes.edgecolor": INK2, "axes.labelcolor": INK,
                     "xtick.color": INK2, "ytick.color": INK2, "svg.fonttype": "none",
                     "svg.hashsalt": "thermoplace"})


def style(ax):
    ax.grid(color=GRID, linewidth=0.8)
    ax.set_axisbelow(True)
    for s in ("top", "right"):
        ax.spines[s].set_visible(False)


# Figure 5: Pareto front
WL = [("sa", "0"), ("sa_t0.25", "0.25"), ("sa_t0.5", "0.5"), ("sa_t0.75", "0.75"), ("sa_t1", "1")]
# Label offsets in points (and alignment), placed by hand so no label sits on a dot or a line.
OFFSETS = {("ami33", "0"): (8, 6, "left"), ("ami33", "0.25"): (6, 8, "left"),
           ("ami33", "0.5"): (-6, 9, "right"), ("ami33", "0.75"): (6, 8, "left"),
           ("ami33", "1"): (0, -17, "center"),
           ("ami49", "0"): (8, -4, "left"), ("ami49", "0.25"): (-8, -4, "right"),
           ("ami49", "0.5"): (0, 9, "center"), ("ami49", "0.75"): (0, 9, "center"),
           ("ami49", "1"): (8, -4, "left")}
fig, axes = plt.subplots(1, 2, figsize=(9, 4.0))
titles = {"ami33": f"ami33: weight 1 lowers Tmax by {a0 - a1:.1f} °C\nfor {ds(P, 'ami33', 'sa_t1') - ds(P, 'ami33', 'sa'):.0f} more points of dead space",
          "ami49": f"ami49: Tmax stays within {max(b_all) - min(b_all):.1f} °C\n(outline leaves only {OUTLINE_FREE['ami49']:.1f} % free)"}
for ax, b in zip(axes, ["ami33", "ami49"]):
    xs, ys = [], []
    for m, wl in WL:
        rs = P[(b, m, "5")]
        dx, dy = col(rs, "dead_space_pct"), col(rs, "max_temp_c")
        ax.scatter(dx, dy, s=14, color=SERIES, alpha=0.25, linewidths=0, zorder=2)
        xs.append(mean(dx)); ys.append(mean(dy))
        if b == "ami33" and wl in ("0.5", "0.75", "1"):
            continue  # three means within 1 point of each other: one shared label below
        dxo, dyo, ha = OFFSETS[(b, wl)]
        ax.annotate(f"w = {wl}", (xs[-1], ys[-1]), textcoords="offset points",
                    xytext=(dxo, dyo), fontsize=9, color=INK2, ha=ha, va="center")
    # ami33 has a front (Tmax falls as w grows); in ami49 the means do not follow w,
    # so joining them in w order would draw a tangle that suggests a trend.
    if b == "ami33":
        ax.plot(xs, ys, color=SERIES, linewidth=2, zorder=3)
        cx, cy = mean(xs[2:]), min(ys[2:])
        ax.annotate("w = 0.5, 0.75, 1", (cx, cy), textcoords="offset points", xytext=(0, -16),
                    fontsize=9, color=INK2, ha="center", va="center")
        ax.set_ylim(min(ys) - 0.35, None)
    ax.scatter(xs, ys, s=48, color=SERIES, edgecolors=SURF, linewidths=2, zorder=4)
    ax.set_title(titles[b], color=INK, fontsize=10.5, loc="left")
    ax.set_xlabel("Dead space (%)")
    ax.set_ylabel("Peak temperature (°C)")
    style(ax)
fig.text(0.01, 0.01, "v1.0, 0.5 to 2 W/mm². Large dots: mean of seeds 1 to 5 for thermal weight w. Faint dots: each seed.",
         fontsize=8.5, color=INK2)
fig.tight_layout(rect=(0, 0.05, 1, 1))
fig.savefig(f"{FIG}/pareto_e13.svg", facecolor=SURF, metadata={"Date": None})
fig.savefig(f"{FIG}/pareto_e13.png", facecolor=SURF, dpi=200)
plt.close(fig)

# Figure 7: SA time vs number of blocks
fig, ax = plt.subplots(figsize=(5.6, 4.0))
ns = [100, 200, 300]
for b, n in zip(("n100", "n200", "n300"), ns):
    ax.scatter([n] * 5, col(G[(b, "sa", "0")], "seconds"), s=14, color=SERIES, alpha=0.3, linewidths=0, zorder=2)
mt = [times[b] for b in ("n100", "n200", "n300")]
ax.plot(ns, mt, color=SERIES, linewidth=2, zorder=3)
ax.scatter(ns, mt, s=48, color=SERIES, edgecolors=SURF, linewidths=2, zorder=4)
for n, t in zip(ns, mt):
    ax.annotate(f"{t:.1f} s", (n, t), textcoords="offset points", xytext=(-10, 9), fontsize=9, color=INK2, ha="right")
ax.set_xscale("log"); ax.set_yscale("log")
ax.set_xticks(ns); ax.set_xticklabels([str(n) for n in ns])
ax.minorticks_off()
yt = [t for t in (1, 2, 5, 10, 20, 50, 100, 200) if min(mt) / 1.6 <= t <= max(mt) * 1.6]
ax.set_yticks(yt); ax.set_yticklabels([str(t) for t in yt])
ax.set_xlim(85, 350)
ax.set_xlabel("Number of blocks (GSRC n100, n200, n300)")
ax.set_ylabel("Mean SA time per run (s, log scale)")
ax.set_title(f"SA time grows as $n^{{{expo:.1f}}}$ on GSRC\n(O(n²) packing × 10·n moves per temperature)",
             color=INK, fontsize=10.5, loc="left")
style(ax)
fig.text(0.01, 0.01, "v1.0, no outline, seeds 1 to 5 (faint dots). Windows 11, MinGW g++ -O2.", fontsize=8.5, color=INK2)
fig.tight_layout(rect=(0, 0.04, 1, 1))
fig.savefig(f"{FIG}/gsrc_time.svg", facecolor=SURF, metadata={"Date": None})
fig.savefig(f"{FIG}/gsrc_time.png", facecolor=SURF, dpi=200)
plt.close(fig)
print("ok")

# ---------------------------------------------------------------- README (optional: argv 6 and 7)
if len(sys.argv) > 7:
    RD_IN, RD_OUT = sys.argv[6], sys.argv[7]
    rd = open(RD_IN, encoding="utf-8").read()
    i, j = rd.index("### Results so far"), rd.index("Reproduce one SA run with")

    def en(x, d):
        return f"{x:,.{d}f}".replace(",", " ")

    R = []
    r_ = R.append
    r_("### Results (v1.0)")
    r_("")
    r_("All numbers below were run with the tag `v1.0` (linear outline penalty, calibrated package), seeds 1 to 5,")
    r_("`--alpha 0.5`. Mean over the 5 seeds; \"fits\" counts the seeds whose floorplan is inside the benchmark's")
    r_("fixed outline. Times are from one Windows 11 PC (MinGW g++ -O2). Full tables, best values, sources and the")
    r_("results with the old quadratic penalty: [docs/RESULTS.md](docs/RESULTS.md).")
    r_("")
    r_("| Benchmark | Method | Dead space | HPWL | Fits | Time (s) |")
    r_("|---|---|---|---|---|---|")
    names = {"random": "random search, 20 000 iterations", "sa": "SA, `--outline-weight 5`",
             "sa_c0.98_m2000": "SA, `--outline-weight 5 --cooling 0.98 --moves-per-temp 2000`"}
    for b in ("ami33", "ami49"):
        for m, ow, _ in E10:
            rs = new["runs"][(b, m, ow)]
            r_(f"| {b} | {names[m]} | {mean(col(rs, 'dead_space_pct')):.2f}% | {en(mean(col(rs, 'hpwl')), 0)} | "
               f"{fits(rs)} | {mean(col(rs, 'seconds')):.2f} |")
    r_("")
    r_("With the linear outline penalty (J18) every SA run fits its outline, including ami49, whose outline leaves")
    r_(f"only {OUTLINE_FREE['ami49']:.1f}% free space. For reference, the original B\\*-tree paper reports 1.27 mm² for ami33")
    r_("and 36.80 mm² for ami49 (Table 2), optimizing area only and without a fixed outline; our best SA runs give")
    r_(f"{float(b33['chip_area']) / 1e6:.3f} mm² and {float(b49['chip_area']) / 1e6:.2f} mm² while also fitting the outline "
       "and optimizing wirelength.")
    r_("")
    r_("**Thermal model.** Against HotSpot v7.0 (block mode) on the best floorplans of ami33 and")
    r_("ami49, the block temperatures of the 16 × 16 grid differ by 0.53 °C on average, with")
    r_("one pair of package values for both benchmarks (h = 1.4e5 W/(m²·K), R = 0.20 K/W). A")
    r_("16 × 16 solve takes under 0.4 ms (`docs/VALIDATION.md`). With much more uneven power")
    r_("(0.1 to 10 W/mm²) the error grows to 1.9 °C on ami33 and 3.6 °C on ami49 (J17).")
    r_("")
    r_(f"**Area against temperature.** With power densities of 0.5 to 2 W/mm², a thermal weight of 1 lowers the")
    r_(f"peak temperature of ami33 by {a0 - a1:.2f} °C for {ds(P, 'ami33', 'sa_t1') - ds(P, 'ami33', 'sa'):.1f} more points of dead space;")
    r_(f"with 0.1 to 10 W/mm², a weight of 0.25 lowers it by {s0 - s25:.2f} °C for "
       f"{ds(S, 'ami33', 'sa_t0.25_p0.1-10') - ds(S, 'ami33', 'sa_p0.1-10'):.1f} points, and HotSpot confirms")
    r_("a drop of 3.7 °C. On ami49 the thermal weight does not lower the peak temperature: its outline leaves too")
    r_("little free space to move the hot blocks apart. Most of the temperature rise comes from the package and does")
    r_("not depend on the floorplan (`docs/RESULTS.md`, E13 and E14).")
    r_("")
    r_(f"**Scalability (GSRC).** SA without outline, seeds 1 to 5: the mean time goes from")
    r_(f"{times['n100']:.2f} s for n100 to {times['n200']:.2f} s for n200 and {times['n300']:.2f} s for n300, about n^{expo:.1f}, because each")
    r_("packing is O(n²) and SA makes 10·n moves per temperature. Mean dead space stays between")
    r_(f"{mean(col(G[('n100', 'sa', '0')], 'dead_space_pct')):.1f}% and {mean(col(G[('n300', 'sa', '0')], 'dead_space_pct')):.1f}%. "
       f"With a fixed outline of 15% free space, n100 fits in {fits(G[('n100', 'sa', '5')])} seeds")
    r_(f"({mean(col(G[('n100', 'sa', '5')], 'dead_space_pct')):.1f}% dead space; 0/5 with the old quadratic penalty) (`docs/RESULTS.md`, E6).")
    r_("")
    rd = rd[:i] + "\n".join(R) + "\n" + rd[j:]
    rd = rd.replace("## Status (week 6, 24 Nov 2026)", "## Status (week 6, 25 Nov 2026, tag `v1.0`)")
    rd = rd.replace("All 57 tests pass on `main`.", "All 57 tests pass on `main` and on `v1.0`.")
    open(RD_OUT, "w", encoding="utf-8", newline="\n").write(rd)
    print("readme ok")
