# Manual de Eimi: lectura de benchmarks, integración y repositorio

**Prioridad 2 de 3.** Sin el lector no corre nada con chips reales (ami33, ami49…), y sin integración las tres partes no se convierten en un programa. Además eres la **integradora**: revisas y haces merge de los Pull Requests y armas el PDF. El repo lo crea Ismael (tarea I0) en su cuenta de GitHub y te agrega como colaboradora, con permiso para hacer merge.

## Lo que te toca

| Archivo | Qué haces |
|---|---|
| `src/Parser.cpp` | Leer los benchmarks MCNC (y GSRC como extra) |
| `app/main.cpp` | Integración y búsqueda aleatoria para la demo (tarea E7) |
| `README.md` | Mantener la tabla de estado y los resultados |
| GitHub | Integrar los Pull Requests de Ismael y Jose (el repo lo crea Ismael) |
| `docs/REVISION1.md` | Coordinar el PDF y los videos |

**No toques** los `.hpp` ni los tests sin acordarlo con el grupo.

Tus tests no dependen de nadie:

```bat
build.bat
build\thermoplace_tests.exe eimi
```

Meta de hoy: `parser: 6/6 PASS`.

## E0. Pasarle el esqueleto a Ismael y clonar (10 min)

Ismael crea el repo en su GitHub (tarea I0 de su manual). Tu parte:

1. Mándale a Ismael el archivo `PROYECTO\ThermoPlace.zip`.
2. Acepta la invitación de colaboradora que te llega por correo desde GitHub.
3. Cuando Ismael confirme que subió el esqueleto, clona el repo en Antigravity (pasos en `docs/ANTIGRAVITY.md`, sección 5) dentro de tu carpeta `PROYECTO`. La carpeta `ThermoPlace_esqueleto` que ya tienes es solo para leer los manuales mientras tanto: **no trabajes ahí**, porque no está conectada a GitHub.

## E1. Verificar que todos compilan (15 min)

Corre `build.bat`. Con los stubs debe compilar y los tests deben fallar, y eso es lo esperado.

Si sale `'g++' no se reconoce`, falta agregar el compilador al PATH. Si tienen Code::Blocks, suele estar en `C:\Program Files\CodeBlocks\MinGW\bin`: agréguenlo en *Variables de entorno → Path* y abran una terminal nueva. Ayuda a quien se trabe en esto: si alguien no compila, no avanza.

## E2. `parseMcncBlocks` (unos 45 min)

Formato de `benchmarks/mcnc/ami33.block`:

```
Outline: 1326 1205
NumBlocks: 33
NumTerminals: 40

bk1   336  133
...
VSS terminal         1410	1610
```

**Ojo:** estos archivos tienen finales de línea de Windows (`\r\n`). Haz `trim()` a **cada** línea. Si no, el nombre queda como `"bk1\r"` y el test `ami33_blocks_and_crlf` falla.

```text
abrir archivo (std::ifstream); si falla -> err = "cannot open " + path; return false
num_linea = 0
por cada línea:
    num_linea++
    line = trim(línea)
    si vacía o empieza con '#': continuar
    t = splitWs(line)
    si empieza con "Outline":
        v = splitWs(line.substr(line.find(':') + 1))      // {"1326", "1205"}
        d.outlineW = stod(v[0]); d.outlineH = stod(v[1])
    si empieza con "NumBlocks" o "NumTerminals": continuar   // (opcional: guardar y verificar al final)
    si t.size() == 4 y t[1] == "terminal":  d.addTerminal(t[0], stod(t[2]), stod(t[3]))
    si t.size() == 3:                       d.addBlock(t[0], stod(t[1]), stod(t[2]))
    si no: err = path + ":" + num_linea + ": unexpected line"; return false
return true
```

Envuelve los `std::stod` en `try { } catch (const std::exception&) { }` y reporta la línea si un número viene mal. `trim`, `splitWs` y `startsWith` ya están en `include/thermoplace/StrUtil.hpp`.

## E3. `parseNets` (unos 30 min)

Formato (igual para MCNC y GSRC):

```
NumNets: 121
NetDegree: 34
GND
bk1
...
```

```text
por cada línea:
    line = trim(línea)
    si NO empieza con "NetDegree": continuar
    k = valueAfterColon(line)
    Net net
    repetir k veces:
        leer la siguiente línea; nombre = primer token (splitWs(trim(..))[0])
        si d.findBlock(nombre) >= 0      -> net.blocks.push_back(...)
        si no, si d.findTerminal(nombre) >= 0 -> net.terminals.push_back(...)
        si no -> err = "unknown pin " + nombre; return false
    d.nets.push_back(net)
```

Valores para revisar: ami33 tiene 121 nets y 425 pines en total.

## E4. `loadBenchmark` (unos 20 min)

```text
d = Design()                      // empezar limpio
separar carpeta y nombre del archivo (busca la última '/' o '\\')
si termina en ".block":       d.name = nombre sin ".block"
                              parseMcncBlocks(path) && parseNets(carpeta + d.name + ".nets")
si termina en ".hardblocks":  (extra E6) parseGsrcHardBlocks, luego parseGsrcPl(.pl), luego parseNets(.nets)
si no: err = "unknown benchmark extension"; return false
```

No uses `std::filesystem`: en el MinGW de Code::Blocks da problemas. Con `find_last_of("/\\")` alcanza.

Prueba: `build\thermoplace_tests.exe eimi` → `parser: 6/6 PASS`

## E5. Integración (cuando lleguen los Pull Requests)

1. En GitHub revisa el PR de Ismael y el de Jose: que **solo** cambien sus archivos.
2. Haz merge a `main`, luego `git pull`, `build.bat` y `build\thermoplace_tests.exe core`.
3. Corre la demo:

   ```bat
   build\thermoplace.exe benchmarks\mcnc\ami33.block --out results
   ```

   Debe decir `Validation: OK (no overlaps)` y escribir `results\ami33_initial.svg` y `.flp`.
4. Copia en el README (sección *Status*) qué funciona y los números de ami33.

## E6. Extra: GSRC (semana 7, para la prueba de escalabilidad)

- `parseGsrcHardBlocks`: líneas como `sb0 hardrectilinear 4 (0, 0) (0, 33) (43, 33) (43, 0)`. El ancho es el mayor x de los 4 puntos y el alto el mayor y (aquí 43 × 33). Las líneas `p1 terminal` se registran con `d.addTerminal("p1")`, que deja la posición en 0.
- `parseGsrcPl`: líneas `p2	4	0` → si `p2` es terminal, asigna x=4, y=0.
- Prueba: `build\thermoplace_tests.exe parser_extra`

## E7. Búsqueda aleatoria para la demo (unos 40 min, después de integrar)

En `app/main.cpp`, dentro del `if (opt.iters > 0)`. Es una versión mínima de lo que en la semana 3 será simulated annealing:

```text
rng = std::mt19937(opt.seed); bestArea = m0.chipArea
repetir opt.iters veces:
    candidate = tree                      // copiar el árbol (es un vector, se copia con =)
    con 50%: candidate.swapBlocks(nodo_al_azar, nodo_al_azar)
    con 50%: rotar un bloque al azar (d.blocks[i].rotated ^= true) y recordar cuál
    candidate.pack(d); area = computeMetrics(d).chipArea
    si area < bestArea: bestArea = area; tree = candidate; mejoras++
    si no: deshacer la rotación (si hubo)
tree.pack(d); validar; imprimir métricas "best"; exportAll(d, opt, "best")
```

Con 20 000 iteraciones en ami33, el espacio muerto debería bajar mucho respecto al inicial (con nuestra implementación de prueba bajó de ~75 % a ~24 %). Ese antes/después es el mejor momento del video.

```bat
build\thermoplace.exe benchmarks\mcnc\ami33.block --out results --iters 20000
```

## E8. Entregables

Sigue `docs/REVISION1.md`: el PDF (plantilla incluida) y que cada uno grabe su video.

## Git

```bash
git checkout -b eimi/parser
git add src/Parser.cpp
git commit -m "Implement MCNC block and net parsers"
git push -u origin eimi/parser
```

Como integras, tú misma haces merge de tu PR después de pasar los tests.

## Checklist

- [ ] Invitación aceptada y repo clonado desde GitHub
- [ ] `build\thermoplace_tests.exe eimi` → `6/6`
- [ ] PRs de Ismael y Jose integrados; `thermoplace_tests core` pasa completo
- [ ] `results\ami33_initial.svg` y `results\ami33_best.svg` generados
- [ ] README actualizado con el estado y los números

## Si usas Antigravity

> Soy Eimi. Lee `AGENTS.md` y `docs/MANUAL_EIMI.md`. Ayúdame con la tarea E2. Solo puedes editar `src/Parser.cpp` (y `app/main.cpp` en la E7). Explícame cada paso antes de escribir el código y al final ejecuta `build.bat` y `build\thermoplace_tests.exe eimi`.
