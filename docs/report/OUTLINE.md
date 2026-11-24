# Informe final: estructura y reparto

**Presentación final:** **viernes 27 de noviembre**. **Entrega del PDF:** jueves 3 de diciembre (en inglés). Para la presentación necesitamos los resultados con `v1.0` y las diapositivas listas el **jueves 26 en la noche**.

**Formato.** Cada sección se escribe en Markdown en `docs/report/` (como ya están `INTRODUCTION.md`, `RELATED_WORK.md` y `METHOD.md`). Ismael las junta en un solo PDF. Extensión objetivo: **8–10 páginas** más referencias y apéndice. Todos los números de resultados salen de corridas con **`v1.0`**.

## Estructura

| # | Sección | Archivo | Responsable | Estado | Contenido |
|---|---|---|---|---|---|
| — | Title, authors, **Abstract** | `ABSTRACT.md` | Ismael | por hacer (al final) | ~200 palabras: problema, método, 3 resultados con número |
| 1 | **Introduction** | `INTRODUCTION.md` | Eimi | borrador listo | Actualizar contribuciones 2–4 con los números de `v1.0`; completar el punto 4 (GSRC) |
| 2 | **Related work** | `RELATED_WORK.md` | Eimi | borrador listo | Revisar la referencia [12] marcada `[CHECK]` |
| 3 | **Method** | `METHOD.md` §1–6 y §8 | Ismael | listo | §8 (Implementation) pasa a ser 3.7 |
| 4 | **Validation against HotSpot** | `VALIDATION_SECTION.md` | Ismael (con números de Jose) | por hacer | Sale de `METHOD.md` §7 + `VALIDATION.md` con `v1.0`: calibración (h, R), error por grilla, potencia desigual, J17 |
| 5 | **Experimental setup** | `SETUP.md` | Jose | por hacer | Benchmarks y outlines (MCNC; GSRC 15 %), potencia (seed, rangos), parámetros de SA, métricas, máquina y tiempos, comandos y `v1.0` |
| 6.1 | **Results: area and wirelength** | `RESULTS_SECTION.md` | Eimi | por hacer | E10: aleatorio vs SA vs SA largo; publicados [B\*-tree, Fast-SA]; **ablación** penalización cuadrática vs lineal |
| 6.2 | **Results: area–temperature trade-off** | `RESULTS_SECTION.md` | Eimi | por hacer | E13 con `v1.0`: tabla + figura de Pareto; base del paquete vs parte que depende de la colocación |
| 6.3 | **Results: power sensitivity** | `RESULTS_SECTION.md` | Jose | por hacer | E14 + J17: el peso térmico baja la Tmax también en HotSpot (ami33); límite en ami49 |
| 6.4 | **Results: scalability** | `RESULTS_SECTION.md` | Eimi | por hacer | E6: tiempo vs n (≈ n^2,7), espacio muerto, n100 al 15 % |
| 7 | **Conclusions and limitations** | `CONCLUSIONS.md` | Ismael | por hacer | Respuesta a la pregunta de investigación, límites, trabajo futuro (contorno O(n), Fast-SA, potencia real) |
| — | **References** | (al juntar) | Eimi | por hacer | Una sola numeración para todo el informe, estilo IEEE |
| A | **Appendix: reproducibility and contributions** | `APPENDIX.md` | Ismael | por hacer | Cómo compilar y repetir cada tabla con `v1.0`; tabla de quién hizo qué |

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
| **mar 24 nov (hoy)** | Une #35; arma la estructura de las diapositivas | Corre E10, E13, E14 y E6 con `v1.0` | — (J15 y J17 ya están, #35) |
| **mié 25 nov** | Sección 4 (Validation), 7 (Conclusions), Abstract; figuras 1 y 2 | **Hasta el mediodía:** `RESULTS.md` con `v1.0` + figuras 5 y 7. Tarde: secciones 6.1, 6.2, 6.4 | Sección 5 (Setup) y 6.3; figuras 3, 4 y 6; números de HPWL de J10 |
| **jue 26 nov** | Mañana: diapositivas completas. Tarde: junta el informe (borrador completo en PDF) | Revisa 4 y 5; diapositivas de resultados | Revisa 1 y 6; diapositivas de validación |
| **jue 26 nov, noche** | **Ensayo completo** de la presentación (los tres, con tiempo) y prueba de la demo | | |
| **vie 27 nov** | **Presentación final** | | |
| sáb 28 – mié 2 dic | Correcciones del informe con lo que comente el profesor; PDF final | Referencias unificadas | Revisión final de cifras |
| **jue 3 dic** | **Entrega del PDF** | | |

## Presentación del viernes 27

Diapositivas en inglés, unos 15 minutos (ajustar al tiempo que dé el profesor), **una idea por diapositiva** con su número.

| # | Diapositiva | Presenta |
|---|---|---|
| 1 | Título, equipo, pregunta de investigación | Ismael |
| 2 | El problema: calor en los chips y floorplanning | Ismael |
| 3 | B\*-tree y empaquetado (figura 2) | Ismael |
| 4 | Simulated annealing y función de costo (figura 1) | Ismael |
| 5 | Modelo térmico en grilla y disipador de dos partes | Ismael |
| 6 | Validación contra HotSpot: error vs. grilla (figura 4) | Jose |
| 7 | Calibración y límite con potencia desigual | Jose |
| 8 | Área y cables vs. búsqueda aleatoria y publicados; penalización lineal | Eimi |
| 9 | Frente de Pareto área vs. Tmax (figura 5) | Eimi |
| 10 | ¿Lo confirma HotSpot? (J17: ami33 sí, ami49 no) | Jose |
| 11 | Escalabilidad en GSRC (figura 7) | Eimi |
| 12 | Demo: ami33 en vivo, floorplan y mapa de calor (figura 3) | Eimi |
| 13 | Conclusiones y límites | Ismael |

Plan B de la demo: capturas de la corrida guardadas en las diapositivas, por si falla la PC.

## Reglas

- **Inglés**, frases cortas, un número con su unidad en cada afirmación de resultados.
- Cada número de resultados debe estar en `RESULTS.md` o `VALIDATION.md` y venir de `v1.0`.
- Cita de otros trabajos: solo lo que dice el paper (y en qué tabla o sección).
- Una rama y un PR por sección (`<nombre>/report-<sección>`); los demás comentan en el PR.
