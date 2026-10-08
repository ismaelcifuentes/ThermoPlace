# Manual de Jose: métricas, verificación y salidas

**Prioridad 3 de 3.** Tu parte convierte el floorplan en algo que se puede **medir, verificar y mostrar**: área, espacio muerto, longitud de cable, la imagen SVG del video y el archivo para HotSpot. Es la parte más independiente: tus tests usan floorplans hechos a mano, así que no esperas a nadie.

## Lo que te toca

| Archivo | Qué haces |
|---|---|
| `src/Metrics.cpp` | `computeMetrics`, `computeHPWL` |
| `src/Validator.cpp` | `validatePlacement` (detectar solapamientos) |
| `src/Exporters.cpp` | `exportSVG`, `exportHotSpotFLP` (y `exportCSV`, `exportPTrace` como extra) |
| `src/Power.cpp` | Extra: potencia sintética reproducible |

**No toques** los `.hpp` ni los tests sin acordarlo con el grupo.

```bat
build.bat
build\thermoplace_tests.exe jose
```

Meta de hoy: `metrics: 4/4`, `validator: 3/3`, `exporters: 3/3`.

Recuerda que cada bloque tiene `x, y` (esquina inferior izquierda) y que el tamaño colocado es `b.w()` × `b.h()`. Usa esos dos, no `width`/`height`, para que la rotación funcione.

## Para empezar

Acepta la invitación de GitHub que te manda Ismael y clona el repo en Antigravity (`docs/ANTIGRAVITY.md`, sección 5). Luego crea tu rama `jose/outputs`.

## J1. `computeMetrics` (unos 20 min)

```text
Metrics m
por cada bloque b:
    m.chipW = max(m.chipW, b.x + b.w())
    m.chipH = max(m.chipH, b.y + b.h())
m.chipArea   = m.chipW * m.chipH
m.blockArea  = d.totalBlockArea()
m.deadSpacePct = (chipArea > 0) ? 100 * (chipArea - blockArea) / chipArea : 0
m.hpwl = computeHPWL(d)
m.fitsOutline = (d.outlineW <= 0 || m.chipW <= d.outlineW) && (d.outlineH <= 0 || m.chipH <= d.outlineH)
```

## J2. `computeHPWL` (unos 30 min)

Es la *Half-Perimeter WireLength*, la medida estándar de cable en floorplanning.

```text
total = 0
por cada net:
    si (blocks.size() + terminals.size()) < 2: continuar
    minX = minY = +infinito; maxX = maxY = -infinito
    por cada bloque k de la net: punto = (k.x + k.w()/2, k.y + k.h()/2)     // centro
    por cada terminal t de la net: punto = (t.x, t.y)
        actualizar min/max con cada punto
    total += (maxX - minX) + (maxY - minY)
return total
```

Usa `std::numeric_limits<double>::max()` (de `<limits>`) como infinito. El test `hpwl` explica el cálculo esperado (20) en un comentario.

## J3. `validatePlacement` (unos 30 min)

```text
r.ok = true
por cada bloque: si width <= 0 o height <= 0 -> error "empty block: <name>"
                 si x < -eps o y < -eps      -> error "negative position: <name>"
por cada par (i < j):
    separadosEnX = a.x + a.w() <= b.x + eps  ||  b.x + b.w() <= a.x + eps
    separadosEnY = a.y + a.h() <= b.y + eps  ||  b.y + b.h() <= a.y + eps
    si !separadosEnX && !separadosEnY -> error "overlap: " + a.name + " and " + b.name
cada error: r.ok = false; r.errors.push_back(mensaje)
```

Que dos bloques se toquen por el borde **no** es solapamiento. Por eso se usa `<=` con `eps`.

## J4. `exportSVG` (unos 45 min, lo que se ve en el video)

```text
std::ofstream out(path); si falla -> err = "cannot write " + path; return false
m = computeMetrics(d)
escala s = 800 / max(m.chipW, m.chipH);   margen pad = 40
<svg xmlns="http://www.w3.org/2000/svg" width=".." height="..">
<text ...>  nombre, área y % de espacio muerto  </text>
<rect ...>  contorno del chip (línea punteada, fill="none") </rect>
por cada bloque:
    xs = pad + b.x * s
    ys = pad + (m.chipH - (b.y + b.h())) * s      // en SVG el eje y apunta HACIA ABAJO
    <rect x=xs y=ys width=b.w()*s height=b.h()*s fill="#9ecae1" stroke="#08519c"/>
    <text x=centro y=centro font-size="10" text-anchor="middle">NOMBRE</text>
</svg>
```

El texto del nombre tiene que quedar como `>A<` (sin espacios) porque el test lo busca así. Abre el `.svg` en el navegador para revisarlo.

## J5. `exportHotSpotFLP` (unos 20 min)

Es el formato verificado con `examples/example1/ev6.flp` de HotSpot v7:

```
# comentario
<nombre>\t<ancho>\t<alto>\t<x-izquierda>\t<y-abajo>
```

Todo en **metros**: multiplica cada valor por `unitToMeters` (por defecto `1e-6`, tratamos las unidades del benchmark como micrómetros). Separa con `\t` y usa `out.precision(9)`.

Con esto, el día que integremos HotSpot (semana 5–6) solo hay que correrlo sobre este archivo. Es lo que respalda la frase "sin rehacer nada" de la propuesta.

## J6. Para el video y el PDF (30 min)

Cuando Eimi integre todo, abre `results\ami33_initial.svg` y `results\ami33_best.svg` en el navegador y toma capturas. Son las imágenes del PDF y de los videos de los tres.

## Extra (semana 5)

### J7. Potencia sintética y `.ptrace`

Los benchmarks **no traen potencia**, y sin potencia no hay modelo térmico. Por eso la asignamos con semilla fija, para que sea reproducible (en el paper se reportan la semilla y el rango).

- `assignSyntheticPower`: `std::mt19937 rng(seed)`, `std::uniform_real_distribution<double> u(minDensity, maxDensity)`, y para cada bloque `power = u(rng) * area()`.
- `exportPTrace`: línea 1 con los nombres separados por `\t`, línea 2 con las potencias separadas por `\t`.
- `exportCSV`: encabezado `name,x,y,w,h,rotated,power` y una fila por bloque.

Prueba: `build\thermoplace_tests.exe extra`

## Git

```bash
git checkout -b jose/outputs
git add src/Metrics.cpp src/Validator.cpp src/Exporters.cpp src/Power.cpp
git commit -m "Implement metrics, validator and SVG/HotSpot exporters"
git push -u origin jose/outputs
```

Después abre un Pull Request hacia `main` y avisa a Eimi.

## Checklist

- [ ] `build\thermoplace_tests.exe jose` → `TOTAL: 10/10`
- [ ] Solo cambiaste tus 4 archivos
- [ ] Abriste un SVG en el navegador y se ve bien (bloques con nombre, título con el área)
- [ ] Puedes explicar qué es HPWL y qué es el espacio muerto

## Si usas Antigravity

> Soy Jose. Lee `AGENTS.md` y `docs/MANUAL_JOSE.md`. Ayúdame con la tarea J1. Solo puedes editar `src/Metrics.cpp`, `src/Validator.cpp`, `src/Exporters.cpp` y `src/Power.cpp`. Explícame cada paso antes de escribir el código y al final ejecuta `build.bat` y `build\thermoplace_tests.exe jose`.
