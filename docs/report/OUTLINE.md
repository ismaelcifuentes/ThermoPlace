# Informe final: estructura y reparto

**Pitch:** viernes 27 de noviembre, 5 minutos, en inglés, lo presenta **solo Eimi**. **Entrega:** jueves 3 de diciembre: el PDF del informe (en inglés) y **un video de 5 minutos por integrante** sobre la implementación de su parte. Cada integrante sube el PDF y su propio video.

**Formato.** Cada sección se escribe en Markdown en `docs/report/` (como ya están `INTRODUCTION.md`, `RELATED_WORK.md` y `METHOD.md`). Ismael las junta en un solo PDF. Extensión objetivo: **8–10 páginas** más referencias y apéndice. Todos los números de resultados salen de corridas con **`v1.0`**.

## Estructura

| # | Sección | Archivo | Responsable | Estado | Contenido |
|---|---|---|---|---|---|
| — | Title, authors, **Abstract** | `ABSTRACT.md` | Ismael | por hacer (29 nov) | ~200 palabras: problema, método, 3 resultados con número |
| 1 | **Introduction** | `INTRODUCTION.md` | Eimi | listo (#42, #43) | Actualizar contribuciones 2–4 con los números de `v1.0`; completar el punto 4 (GSRC) |
| 2 | **Related work** | `RELATED_WORK.md` | Eimi | listo salvo la ref. [12] | Revisar la referencia [12] marcada `[CHECK]` |
| 3 | **Method** | `METHOD.md` §1–6 y §8 | Ismael | listo | §8 (Implementation) pasa a ser 3.7 |
| 4 | **Validation against HotSpot** | `VALIDATION_SECTION.md` | Ismael (con números de Jose) | listo (#39) | Sale de `METHOD.md` §7 + `VALIDATION.md` con `v1.0`: calibración (h, R), error por grilla, potencia desigual, J17 |
| 5 | **Experimental setup** | `SETUP.md` | Jose | listo (#40) | Benchmarks y outlines (MCNC; GSRC 15 %), potencia (seed, rangos), parámetros de SA, métricas, máquina y tiempos, comandos y `v1.0` |
| 6.1 | **Results: area and wirelength** | `RESULTS_SECTION.md` | Eimi | listo (#41) | E10: aleatorio vs SA vs SA largo; publicados [B\*-tree, Fast-SA]; **ablación** penalización cuadrática vs lineal |
| 6.2 | **Results: area–temperature trade-off** | `RESULTS_SECTION.md` | Eimi | listo (#41) | E13 con `v1.0`: tabla + figura de Pareto; base del paquete vs parte que depende de la colocación |
| 6.3 | **Results: power sensitivity** | `RESULTS_63.md` (se junta al armar el PDF) | Jose | listo (#40, #44) | E14 + J17: el peso térmico baja la Tmax también en HotSpot (ami33); límite en ami49 |
| 6.4 | **Results: scalability** | `RESULTS_SECTION.md` | Eimi | listo (#41) | E6: tiempo vs n (≈ n^2,7), espacio muerto, n100 al 15 % |
| 7 | **Conclusions and limitations** | `CONCLUSIONS.md` | Ismael | listo (#39, #43, #45) | Respuesta a la pregunta de investigación, límites, trabajo futuro (contorno O(n), Fast-SA, potencia real) |
| — | **References** | (al juntar) | Eimi | por hacer (28 nov – 2 dic) | Una sola numeración para todo el informe, estilo IEEE |
| A | **Appendix: reproducibility and contributions** | `APPENDIX.md` | Ismael | por hacer (29 nov) | Cómo compilar y repetir cada tabla con `v1.0`; tabla de quién hizo qué |

## Figuras

| Fig. | Qué muestra | Sección | Responsable |
|---|---|---|---|
| 1 | El ciclo del método: B\*-tree → empaquetado → costo → SA | 3 | Ismael |
| 2 | Ejemplo de B\*-tree y su floorplan (el diagrama del video de la parte 1) | 3 | Ismael |
| 3 | Floorplan final de ami33 y su mapa de calor, lado a lado | 3 o 6 | Jose |
| 4 | Error contra HotSpot según el tamaño de la grilla | 4 | Jose |
| 5 | Frente de Pareto área vs. Tmax (`docs/figures/pareto_e13.svg` con `v1.0`) | 6.2 | Eimi |
| 6 | Curva de convergencia del SA (ami33) | 5 o 6.1 | Jose |
| 7 | Tiempo de ejecución vs número de bloques (GSRC) | 6.4 | Eimi |

Las figuras van en `docs/figures/` (SVG o PNG), con un título que diga el resultado, no solo el tema.

## Calendario

| Fecha | Ismael | Eimi | Jose |
|---|---|---|---|
| **vie 27 nov** | **Pitch** (presenta Eimi, 5 min) | **Pitch**; plan B en USB: PDF del Canva y GIF de la demo | **Pitch**; cronometra y prepara preguntas |
| sáb 28 nov | Guion de su video; arreglos de `METHOD.md` | Guion de su video; ref. [12] | Guion de su video |
| dom 29 nov | Abstract, Appendix, figuras 1 y 2; revisa los guiones; **junta el PDF** | Referencias IEEE unificadas (con la cita de MCNC y GSRC) | Revisión de cifras del PDF |
| 30 nov – 2 dic | Graba su video; PDF final | Graba su video | Graba su video |
| **jue 3 dic** | **Entrega:** cada uno sube el PDF y su video | | |

## Pitch del viernes 27 (5 min, Eimi)

Canva "ThermoPlace Pitch", 7 diapositivas, unos 40 s cada una. Solo números de `v1.0` que estén en el informe.

| # | Diapositiva | Mensaje |
|---|---|---|
| 1 | Gancho | Chips don't run out of transistors. They run out of cooling. |
| 2 | Qué es ThermoPlace | B\*-tree + SA + modelo térmico rápido (figura 3) |
| 3 | Validación | 0.53 °C from HotSpot, at least 40× faster (figura 4) |
| 4 | Floorplans | 1.245 vs 1.27 mm² en ami33; cabe en el outline sin término térmico |
| 5 | Temperatura | −3.7 °C en el hot spot de ami33, confirmado por HotSpot (figura 3b); ami49 no baja |
| 6 | Velocidad | ami33 en ~11 s con temperatura; 300 bloques en 155 s |
| 7 | Cierre | Open. Tested. Validated. Repo y equipo |

## Videos del 3 de diciembre (5 min cada uno, en inglés)

| Quién | Su parte |
|---|---|
| Ismael | B\*-tree y contorno, simulated annealing, modelo térmico y calibración del paquete |
| Eimi | Lector de benchmarks, `main.cpp` y la CLI, experimentos E10/E13/E14/E6, figuras 5 y 7 |
| Jose | Función de costo y métricas, potencia, exportación a HotSpot y validación J15/J17, figuras 3, 4 y 6 |

## Reglas

- **Inglés**, frases cortas, un número con su unidad en cada afirmación de resultados.
- Cada número de resultados debe estar en `RESULTS.md` o `VALIDATION.md` y venir de `v1.0`.
- Cita de otros trabajos: solo lo que dice el paper (y en qué tabla o sección).
- Una rama y un PR por sección (`<nombre>/report-<sección>`); los demás comentan en el PR.
