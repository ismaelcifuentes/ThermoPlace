# Cómo trabajamos en el repositorio

## Reglas

1. **Cada archivo tiene un dueño** (la tabla está en `AGENTS.md`). Solo editas los tuyos.
2. **Los `.hpp` de `include/thermoplace/` son el contrato** entre los tres. Si hay que cambiarlos, primero se habla en el grupo.
3. **Los tests no se tocan** para que pasen. Si crees que un test está mal, avisa.
4. **Cada persona trabaja en su rama** y entra a `main` con un Pull Request. El repo es de Ismael (líder); Eimi revisa y hace los merge.

## Primera vez (Eimi y Jose; Ismael ya lo tiene desde la tarea I0)

Paso a paso con Antigravity en `docs/ANTIGRAVITY.md`, sección 5. Por terminal sería:

```bash
git clone https://github.com/ismaelcifuentes/ThermoPlace.git
cd ThermoPlace
build.bat                       # debe compilar; los tests fallan porque falta implementar
git checkout -b jose/outputs    # o eimi/parser
```

## Ciclo de trabajo

```bash
# ... editas tus archivos ...
build.bat
build\thermoplace_tests.exe ismael        # tus tests
git add src/Contour.cpp src/BStarTree.cpp # SOLO tus archivos
git commit -m "I1: contour"
git push                                  # la primera vez: git push -u origin ismael/bstar
```

Cuando tus tests pasen: en GitHub entra a **Pull requests → New pull request** (de tu rama a `main`) y avisa en el grupo.

## Traer lo último de main (después de cada merge)

```bash
git checkout main
git pull
git checkout ismael/bstar
git merge main
```

Como cada uno edita archivos distintos, no debería haber conflictos.

## Si no conocen la terminal

GitHub Desktop o el panel de Git de Antigravity / VS Code hacen lo mismo: *Clone → New branch → Commit → Push → Create Pull Request*.
