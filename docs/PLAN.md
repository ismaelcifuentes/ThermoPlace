# Plan de trabajo y reparto de tareas

## Orden de prioridad y asignación

| Prioridad | Rol | Persona | Por qué va en ese lugar |
|---|---|---|---|
| 1 | **B\*-tree y empaquetado** (antes "Persona B") + **líder y dueño del repo** | **Ismael** | Es el núcleo del algoritmo y lo más difícil. El simulated annealing, el modelo térmico y el paper dependen de él. Si falla, no hay proyecto. Además el repo vive en su GitHub (I0). |
| 2 | **Lector e integración** (antes "Persona A") | **Eimi** | Sin el lector no corre nada con chips reales, y sin integración no hay programa. Es quien revisa y hace merge de los Pull Requests. |
| 3 | **Métricas, verificación y salidas** (antes "Persona C") | **Jose** | Es necesaria para medir y mostrar, pero es la parte más simple y más independiente: se prueba con floorplans hechos a mano. |

Las tres partes se trabajan **en paralelo desde el minuto 1**. Los `.hpp` de `include/thermoplace/` ya definen qué recibe y qué devuelve cada función, y cada persona tiene sus propios tests.

## Hoy: revisión 1 (unas 4–5 horas de trabajo enfocado)

| Hora | Ismael | Eimi | Jose |
|---|---|---|---|
| antes | Instalar Git, compilador y Antigravity (`docs/ANTIGRAVITY.md`) | igual | igual |
| 0:00–0:20 | **I0** crear repo, subir esqueleto, agregar a Eimi y Jose | **E0** pasar el zip a Ismael; luego clonar; **E1** ayudar a que todos compilen | Aceptar invitación, clonar, `build.bat`, leer manual |
| 0:20–1:00 | **I1** Contour, **I2** buildInitial | **E2** parseMcncBlocks | **J1** computeMetrics, **J2** HPWL |
| 1:00–2:30 | **I3** pack, **I5** swapBlocks | **E3** parseNets, **E4** loadBenchmark | **J3** validator, **J4** SVG, **J5** FLP |
| 2:30–3:00 | **I4** isValid/toString → PR | PR propio + revisar PRs | PR |
| 3:00–3:45 | **I6** diagrama del árbol para el video | **E5** integrar y correr ami33; **E7** búsqueda aleatoria | **J6** capturas de los SVG |
| 3:45– | Video | PDF (`docs/REVISION1.md`) + video | Video |

**Punto de control 2:30:** los tres grupos de tests en verde (`thermoplace_tests core`).
Si alguien está trabado a las 2:00, avise al grupo **ya**; no lo dejen para el final.

## Semanas 3–8 (hacia la revisión 2 y el paper)

| Semana | Ismael | Eimi | Jose |
|---|---|---|---|
| 3 | `moveNode` (I7) + motor de **simulated annealing** (temperatura, enfriamiento, aceptación de Metropolis) | Opciones de línea de comandos para SA, salida CSV por corrida | Función de costo: área + α·HPWL normalizados; gráficas de convergencia |
| 4 | Ajuste de SA (perturbaciones, criterio de parada) | **Comparación con resultados publicados** de ami33/ami49 (tabla) | Gráficas de floorplans finales; revisión de que HPWL coincide con la definición de los papers |
| 5 | **Modelo térmico en matriz**: grilla N×N, potencia por celda, difusión de calor resuelta con Jacobi/Gauss-Seidel → temperatura máxima | Corridas con y sin temperatura (mismas semillas) | **J7** potencia sintética + `.ptrace`; instalar y correr **HotSpot** con los `.flp` exportados |
| 6 | Temperatura dentro del costo de SA | Barrido del peso térmico → **frente de Pareto área vs. Tmax** | **Validación del modelo rápido contra HotSpot** (error y tiempo según tamaño de grilla) |
| 7 | Contour O(n) con lista enlazada + medición de tiempo | **E6** GSRC n100–n300: escalabilidad | Figuras para el paper |
| 8 | Paper: sección *Method* | Paper: *Introduction*, *Related work*, edición final | Paper: *Experiments*, *Results* |

## La contribución del paper (para no perder el foco)

> *¿Qué tan simple puede ser el modelo térmico dentro del optimizador sin perder precisión frente a HotSpot, y cuánta área cuesta cada grado de temperatura máxima?*

Resultados que debemos poder mostrar al final:

1. una tabla de área y HPWL en ami33/ami49 comparada con resultados publicados,
2. un frente de Pareto de área contra Tmax,
3. una curva de error del modelo rápido contra HotSpot y su tiempo de cálculo,
4. el tiempo de ejecución en n100–n300.

Destinos posibles: INCISCOS / ETCM / CLEI 2027, o el ACM Student Research Competition (abstract de 800 palabras).
