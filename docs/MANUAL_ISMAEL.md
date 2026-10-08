# Manual de Ismael: B*-tree y empaquetado

**Prioridad 1 de 3, y líder del proyecto.** Además de tu parte técnica, el repositorio vive en tu cuenta de GitHub (tarea I0). Este es el corazón del proyecto. Sin el empaquetado no hay floorplan, y todo lo que viene después (simulated annealing en la semana 3, modelo térmico en la 5 y el paper) se construye sobre este árbol. Es también la parte más difícil, por eso va primero.

## Lo que te toca

| Archivo | Qué haces |
|---|---|
| `src/Contour.cpp` | Implementar la clase `Contour` (el "skyline" de bloques ya colocados) |
| `src/BStarTree.cpp` | Implementar `buildInitial`, `pack`, `swapBlocks`, `isValid`, `toString` (y `moveNode` como extra) |

**No toques** los `.hpp` de `include/thermoplace/`. Son el contrato con Eimi y Jose; si necesitas cambiar algo, avísale al grupo primero. Tampoco modifiques los tests.

Tus tests no dependen de nadie: construyen los bloques a mano. Puedes trabajar desde el minuto uno.

```bat
build.bat
build\thermoplace_tests.exe ismael
```

Meta de hoy: `contour: 2/2 PASS` y `bstar: 8/8 PASS`.

## I0. Crear el repositorio (lo primero de todo, 15–20 min)

Los demás no pueden clonar hasta que esto esté listo. Necesitas Git y Antigravity instalados (`docs/ANTIGRAVITY.md`).

1. Descomprime `ThermoPlace.zip` (te lo pasa Eimi) dentro de tu carpeta de trabajo, por ejemplo `Documentos\PROYECTO\ThermoPlace`.
2. En github.com: **+ → New repository** → nombre `ThermoPlace` → **Public** → **no** marques *Add a README*, *.gitignore* ni *license* (ya vienen en el esqueleto) → **Create repository**.
3. En Antigravity: **File → Open Folder** → la carpeta `ThermoPlace`. Abre la terminal (**Ctrl+Ñ** o **Terminal → New Terminal**) y ejecuta:

   ```bat
   git init
   git add .
   git commit -m "Project skeleton: interfaces, stubs, tests, benchmarks"
   git branch -M main
   git remote add origin https://github.com/ismaelcifuentes/ThermoPlace.git
   git push -u origin main
   ```

   La primera vez que hagas `push` se abre el navegador para iniciar sesión en GitHub: acepta.
4. En GitHub: **Settings → Collaborators → Add people** → agrega a Eimi y a Jose (con su usuario o correo). Al aceptar la invitación quedan con permiso de escritura, así Eimi puede hacer merge de los Pull Requests.
5. Manda el link al grupo: *"ya está el repo, clónenlo siguiendo docs/ANTIGRAVITY.md, sección 5"*.
6. Crea tu rama y empieza: `git checkout -b ismael/bstar`.

## Primero, entiende el B*-tree (10 min)

Cada nodo del árbol tiene un bloque. Las reglas son:

- La **raíz** va en `(0, 0)`.
- El **hijo izquierdo** de `n` va **a la derecha** de `n`: `x = x(n) + w(n)`.
- El **hijo derecho** de `n` va **encima** de `n`, con el mismo x: `x = x(n)`.
- La **y** de cada bloque es la altura máxima del contour en el intervalo `[x, x + w)`. El bloque "cae" hasta tocar lo que ya hay debajo.
- Se recorre en **preorden**: el nodo, luego **todo** su subárbol izquierdo y después el derecho. El orden importa: un hijo derecho ancho puede quedar encima de bloques del subárbol izquierdo de su padre, así que esos tienen que estar colocados antes.

Ejemplo del test `pack_three_blocks`: A(4×2) raíz, B(2×2) hijo izquierdo, C(4×1) hijo derecho.

```
y=3 +-------+
    |   C   |
y=2 +-------+---+
    |   A   | B |
y=0 +-------+---+
    x=0     4   6
```

A queda en (0,0). B va a la derecha de A, así que x=4, y=0. C va encima de A con x=0; el contour en [0,4) mide 2, así que y=2.

Lectura recomendada (solo la sección del B*-tree): Chen y Chang, *Modern Floorplanning Based on B\*-Tree and Fast Simulated Annealing*, IEEE TCAD 2006 — https://cc.ee.ntu.edu.tw/~ywchang/Papers/tcad06-mfloorplanning.pdf

## Tareas de hoy (en este orden)

### I1. `Contour` (unos 20 min) — `src/Contour.cpp`

Guardamos una lista de segmentos `{x1, x2, top}`, uno por bloque colocado.

- `clear()`: vacía `segs_`.
- `insert(x1, x2, top)`: `segs_.push_back({x1, x2, top});`
- `maxHeight(x1, x2)`: recorre los segmentos. Un segmento `s` **se superpone** con `(x1, x2)` si `s.x1 < x2 && x1 < s.x2`. Devuelve el mayor `top` entre los que se superponen, o `0.0` si no hay ninguno. Si dos segmentos solo se tocan en un punto (`s.x2 == x1`), **no** cuentan.

Prueba: `build\thermoplace_tests.exe contour`

> Para el paper (semana 7): esta versión es O(n) por consulta, O(n²) por empaquetado. Con una lista doblemente enlazada se llega a O(n) por empaquetado (Chang 2000). Medir esa diferencia sirve como resultado.

### I2. `buildInitial(n)` (unos 20 min)

Es un árbol binario completo: el nodo `i` tiene el bloque `i`, su hijo izquierdo es `2i+1` y el derecho `2i+2`, siempre que existan (`< n`).

```text
nodes_.assign(n, Node{})
root_ = (n > 0) ? 0 : -1
para i en 0..n-1:
    nodes_[i].block = i
    si i > 0:        nodes_[i].parent = (i - 1) / 2
    si 2i+1 < n:     nodes_[i].left   = 2i+1
    si 2i+2 < n:     nodes_[i].right  = 2i+2
```

### I3. `pack(Design& d)` (1 a 1.5 h, la parte central)

Usa una pila (`std::vector<int>`) para hacer el preorden sin recursión. La recursión también funciona, pero con n=300 es más seguro iterar.

```text
si root_ == -1: return
Contour c; c.clear()
pila = [root_]
mientras la pila no esté vacía:
    n = pila.back(); pila.pop_back()
    Block& b = d.blocks[ nodes_[n].block ]
    p = nodes_[n].parent
    si p == -1:  x = 0
    si no:
        const Block& pb = d.blocks[ nodes_[p].block ]
        si nodes_[p].left == n:  x = pb.x + pb.w()    // hijo izquierdo -> a la derecha
        si no:                   x = pb.x             // hijo derecho   -> encima
    b.x = x
    b.y = c.maxHeight(x, x + b.w())
    c.insert(x, x + b.w(), b.y + b.h())
    // primero se apila el derecho y luego el izquierdo, así el izquierdo sale primero
    si nodes_[n].right != -1: pila.push_back(nodes_[n].right)
    si nodes_[n].left  != -1: pila.push_back(nodes_[n].left)
```

Usa siempre `b.w()` y `b.h()`, nunca `width`/`height`: así la rotación funciona sola.

Prueba: `build\thermoplace_tests.exe bstar`. El test `pack_uses_contour_of_neighbours` falla si el orden del recorrido está mal. `random_packings_never_overlap` empaqueta 60 bloques 200 veces y revisa que no haya solapamientos.

### I4. `isValid()` y `toString()` (unos 40 min)

`isValid()` devuelve true si:

1. el árbol está vacío y `root_ == -1`, **o**
2. `root_` está en rango y su `parent == -1`,
3. al recorrer desde la raíz cada nodo aparece **una sola vez** (usa un `std::vector<int> visto(n)`),
4. para cada hijo `c` de `v`, `nodes_[c].parent == v`,
5. cada bloque `0..n-1` aparece en exactamente un nodo,
6. no queda ningún nodo sin visitar.

`toString()`: una línea por nodo con este formato:
`node 0 block 0 parent -1 left 1 right 2`
Usa `std::ostringstream`.

### I5. `swapBlocks(a, b)` (5 min)

`std::swap(nodes_[a].block, nodes_[b].block);` La forma del árbol no cambia; solo se intercambian los bloques. Es una de las tres perturbaciones del simulated annealing.

### I6. Para el video (30 min)

Dibuja (a mano o en draw.io) el árbol de 3 o 4 bloques del test `pack_uses_contour_of_neighbours` junto a su floorplan. En tu video explica en inglés: *"each node is a block; the left child goes to the right, the right child goes on top; y comes from the contour"*.

## Extra (semana 3)

### I7. `moveNode(node, newParent, asLeftChild)`

Versión simple: solo mueve **hojas**.

- Devuelve `false` si `node` es la raíz, si `node == newParent`, si `node` tiene hijos o si el espacio (left o right) de `newParent` ya está ocupado.
- Si se puede: en el padre actual pon `left` o `right` en `-1` (según dónde estaba), coloca `node` en el espacio libre de `newParent` y actualiza `nodes_[node].parent`.

Prueba: `build\thermoplace_tests.exe bstar_extra`

## Git (tu rama)

```bash
git checkout -b ismael/bstar      # si no la creaste en I0
# ... trabajas ...
git add src/Contour.cpp src/BStarTree.cpp
git commit -m "Implement contour and B*-tree packing"
git push -u origin ismael/bstar
```

Luego abre un Pull Request hacia `main` en GitHub y avisa a Eimi para que lo integre. Haz commits pequeños (uno por tarea I1, I2…). Así en el video se ve el avance.

## Checklist antes de avisar "listo"

- [ ] Repo creado, esqueleto en `main`, Eimi y Jose agregados (I0)
- [ ] `build.bat` compila sin errores
- [ ] `build\thermoplace_tests.exe ismael` → `TOTAL: 10/10`
- [ ] Solo cambiaste `src/Contour.cpp` y `src/BStarTree.cpp`
- [ ] Puedes explicar en voz alta, sin leer, cómo `pack` calcula x e y

## Si usas Antigravity

Pega esto al agente:

> Soy Ismael. Lee `AGENTS.md` y `docs/MANUAL_ISMAEL.md`. Ayúdame con la tarea I1. Solo puedes editar `src/Contour.cpp` y `src/BStarTree.cpp`. Explícame cada paso antes de escribir el código y, al terminar, ejecuta `build.bat` y `build\thermoplace_tests.exe ismael`.

Pídele que te explique, no solo que escriba: en el video tienes que defender este código.
