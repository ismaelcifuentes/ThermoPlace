# Resultados en ami33 y ami49 (semana 3)

Corridas de E10 con el código de `main` después del PR #11 (I9 e I10). Semillas 1 a 5, `--alpha 0.5`. Máquina: PC de Eimi, Windows 11, g++ de MinGW con -O2 (build.bat). Los datos crudos están en `results/runs.csv`, que no se sube al repo; este archivo se generó a partir de él.

Cómo leer las tablas:

- **Espacio muerto** = 1 − (área de los bloques / área del chip).
- **Mejor** es el mínimo de esa columna entre las 5 semillas; el mejor espacio muerto y el mejor HPWL pueden venir de semillas distintas.
- **Cabe** cuenta las semillas cuyo resultado entra en el outline fijo del benchmark.
- La búsqueda aleatoria solo minimiza área y no sabe del outline; en `runs.csv` aparece con `outline_weight` 0.

## ami33

| Método | Corridas | Espacio muerto medio | Mejor espacio muerto | HPWL medio | Mejor HPWL | Área media (mm²) | Cabe | Tiempo medio (s) |
|---|---|---|---|---|---|---|---|---|
| Búsqueda aleatoria, 20 000 iteraciones | 5 | 23,27 % | 18,17 % | 146 156 | 130 754 | 1,509 | 0/5 | 0,15 |
| SA, `--outline-weight 5` | 5 | 15,63 % | 10,93 % | 92 976 | 86 240 | 1,372 | 5/5 | 0,59 |
| SA largo, `--outline-weight 5 --cooling 0.98 --moves-per-temp 2000` | 5 | 12,77 % | 9,81 % | 84 265 | 81 447 | 1,326 | 5/5 | 9,77 |

## ami49

| Método | Corridas | Espacio muerto medio | Mejor espacio muerto | HPWL medio | Mejor HPWL | Área media (mm²) | Cabe | Tiempo medio (s) |
|---|---|---|---|---|---|---|---|---|
| Búsqueda aleatoria, 20 000 iteraciones | 5 | 26,11 % | 12,92 % | 2 107 536 | 1 923 551 | 48,411 | 0/5 | 0,27 |
| SA, `--outline-weight 5` | 5 | 11,90 % | 11,04 % | 1 020 173 | 975 681 | 40,237 | 3/5 | 1,67 |
| SA largo, `--outline-weight 5 --cooling 0.98 --moves-per-temp 2000` | 5 | 10,80 % | 9,42 % | 906 049 | 774 067 | 39,747 | 2/5 | 17,06 |

## Resultados publicados

| Benchmark | Trabajo | Área (mm²) | Espacio muerto | Tiempo | De dónde sale |
|---|---|---|---|---|---|
| ami33 | B*-tree, SA [1] | 1,27 | 8,94 % (calculado) | 3417 s | [1], Table 2, columnas "SA B*-tree" |
| ami49 | B*-tree, SA [1] | 36,80 | 3,68 % (calculado) | 4752 s | [1], Table 2, columnas "SA B*-tree" |
| ami33 | Fast-SA [2] | no reportado | no reportado | no reportado | [2] no trae ami33 con optimización de área |
| ami49 | Fast-SA [2] | no reportado | 2,00 % | ver Table II | [2], texto de la Fig. 15 y Table II (p. 646) |

Notas sobre esta tabla:

- [1] optimiza **solo área**, sin outline fijo y sin wirelength; no reporta HPWL. Corrió en una SUN Sparc Ultra-I de 200 MHz, así que sus tiempos no se comparan con los nuestros.
- [1] no indica la unidad de área. Asumimos mm² porque las dimensiones de MCNC están en µm: el área de bloques de ami33 es 1 156 449 µm² = 1,156 mm², coherente con 1,27 mm². El espacio muerto "calculado" sale de esa suposición, no del paper.
- [2] reporta ami49 en la comparación de esquemas de SA (greedy, SA clásico, TimberWolf, Fast-SA), con optimización de área, en un Pentium 4 de 1,6 GHz. El 2,00 % es el valor final que da el texto al comentar la Fig. 15; los tiempos de la Table II hay que copiarlos del PDF, porque no se pudieron extraer del texto.
- Ni [1] ni [2] reportan HPWL para ami33/ami49 sin outline, así que no hay HPWL publicado contra el cual comparar el nuestro.

Referencias:

1. Y.-C. Chang, Y.-W. Chang, G.-M. Wu y S.-W. Wu, "B*-Trees: A New Representation for Non-Slicing Floorplans", en *Proc. ACM/IEEE Design Automation Conference (DAC)*, 2000, pp. 458–463.
2. T.-C. Chen y Y.-W. Chang, "Modern Floorplanning Based on B*-Tree and Fast Simulated Annealing", *IEEE Trans. Computer-Aided Design of Integrated Circuits and Systems*, vol. 25, no. 4, pp. 637–650, abr. 2006.

## Definición de HPWL (Jose, J10)

Pendiente: Jose anota aquí cómo define HPWL [2] y si coincide con la nuestra (centro de los bloques, terminales incluidos).
