# Instalar y configurar el entorno (Windows): Git, compilador y Antigravity IDE

Los tres hacen las secciones 1 a 4. Ismael hace después la tarea I0 de su manual (crear el repo), y Eimi y Jose siguen con la sección 5 (clonar). Tiempo total: unos 30–40 minutos, casi todo descargas.

> **Importante:** Google tiene dos productos con nombre parecido. **Antigravity 2.0** es una app solo de agentes, **sin editor de código**. Nosotros usamos el **Antigravity IDE** (el editor con agente, basado en VS Code). Descarguen ese.

---

## 1. Git (5 min)

1. Descarguen *Git for Windows* desde https://git-scm.com/download/win (*64-bit Git for Windows Setup*).
2. Instálenlo dejando **todas las opciones por defecto**. Ya incluye *Git Credential Manager*, que es lo que permite iniciar sesión en GitHub desde el navegador.
3. Abran **Símbolo del sistema** (cmd) y configuren su nombre y el correo de su cuenta de GitHub:

   ```bat
   git config --global user.name "Nombre Apellido"
   git config --global user.email "correo-de-su-github@ejemplo.com"
   git --version
   ```

## 2. Compilador C++ (10–15 min)

Elijan **una** opción.

### Opción A (recomendada): MSYS2

1. Descarguen el instalador desde https://www.msys2.org e instálenlo en la ruta por defecto `C:\msys64`.
2. Al terminar se abre una terminal de MSYS2. Abran desde el menú inicio **"MSYS2 UCRT64"** y ejecuten (acepten con Enter/`Y`):

   ```bash
   pacman -S --needed mingw-w64-ucrt-x86_64-toolchain mingw-w64-ucrt-x86_64-clang-tools-extra
   ```

   Eso instala `g++`, `gdb` y `clangd` (este último sirve para el autocompletado en Antigravity).
3. Agreguen el compilador al PATH: tecla Windows → escriban *"variables de entorno"* → **Editar las variables de entorno de esta cuenta** → seleccionen **Path** → **Editar** → **Nuevo** → `C:\msys64\ucrt64\bin` → **Aceptar** en todas las ventanas.

### Opción B (si ya tienen Code::Blocks con MinGW)

Agreguen al PATH (mismo procedimiento del paso 3) la carpeta `C:\Program Files\CodeBlocks\MinGW\bin`. Revisen que exista y que tenga `g++.exe`. Funciona con nuestro proyecto (C++17), pero no trae `clangd`; en ese caso salten el paso 4.4.

### Verificar

Abran una **cmd nueva** (las que ya estaban abiertas no ven el PATH nuevo) y ejecuten:

```bat
g++ --version
```

Si dice `"g++" no se reconoce…`, el PATH quedó mal: revisen la ruta.

## 3. Cuenta de GitHub (2 min)

Si alguien no tiene cuenta: https://github.com/signup. Pásenle su **usuario** a Ismael para que los agregue como colaboradores.

## 4. Antigravity IDE (10 min)

1. Entren a https://antigravity.google/download y, en la sección **Antigravity IDE** (no la de *Antigravity 2.0*), descarguen el instalador **Windows x64** (`.exe`). Requiere Windows 10 de 64 bits o superior.
2. Ejecútenlo. Si Windows pregunta si confía en el programa, revisen que el editor sea Google y acepten.
3. **Primer arranque:**
   - Inicien sesión con su **cuenta de Google**. Si la cuenta institucional da error (las administra la universidad y a veces bloquean estos servicios), usen una Gmail personal.
   - Pueden **importar la configuración de VS Code** si lo usaban, o empezar de cero.
   - Si pregunta cuánta autonomía darle al agente, elijan la opción en la que **el agente pide revisión/aprobación** antes de ejecutar comandos o cambios grandes (*review-driven*). Así nada se ejecuta sin que lo vean.
   - En lo demás, dejen los valores por defecto.
4. **Extensión de C++:** abran Extensiones (**Ctrl+Shift+X**), busquen **clangd** (publicada por *LLVM*) e instálenla.
   - **No** sirve la extensión *C/C++* de Microsoft: desde 2025 solo funciona en el VS Code oficial, no en editores derivados como Antigravity.
   - Si usaron MSYS2: **Ctrl+,** (Configuración) → busquen `clangd.path` → escriban `C:\msys64\ucrt64\bin\clangd.exe`. Si la extensión ofrece descargar su propio clangd, digan que no.
   - El repo ya trae `compile_flags.txt`, con el que clangd sabe que usamos C++17 y dónde están los `.hpp`.
5. **Terminal por defecto en cmd**, para que los comandos de los manuales funcionen tal cual: **Ctrl+Shift+P** → escriban `Terminal: Select Default Profile` → **Command Prompt**.
6. **Avisos de actualización:** si Antigravity ofrece actualizar o pasar a *Antigravity 2.0*, elijan **mantener el IDE**. Hay reportes de que la actualización reemplazó el IDE por la app sin editor.

## 5. Clonar el repo (Eimi y Jose, cuando Ismael termine I0)

1. Acepten la invitación de colaborador que GitHub les manda por correo.
2. En Antigravity: **Ctrl+Shift+P** → `Git: Clone` → peguen `https://github.com/ismaelcifuentes/ThermoPlace.git` → elijan la carpeta `PROYECTO` → **Open** cuando pregunte si abrir el repo.
3. Si pide iniciar sesión en GitHub, acepten en el navegador.
4. Abran la terminal (**Terminal → New Terminal**) y compilen:

   ```bat
   build.bat
   build\thermoplace_tests.exe
   ```

   Que compile y que **fallen** los tests es lo esperado: falta implementar.
5. Creen su rama: clic en **`main`** abajo a la izquierda → **Create new branch** → `eimi/parser` o `jose/outputs`.

## 6. El día a día en Antigravity

| Qué | Cómo |
|---|---|
| Compilar y probar | Terminal: `build.bat` y luego `build\thermoplace_tests.exe <su-nombre>` |
| Ver cambios y commitear | Panel de Git (**Ctrl+Shift+G**) → **+** solo en **sus** archivos → mensaje (`I1: contour`) → **Commit** |
| Subir | Botón **Sync Changes** / **Publish Branch** |
| Pull Request | En github.com aparece **Compare & pull request** → hacia `main` → **Create**. Avisen a Eimi. |
| Traer lo último de `main` | Panel de Git → `…` → **Pull**; o en la terminal: `git checkout main`, `git pull`, `git checkout <su-rama>`, `git merge main` |

## 7. Cómo usar el agente sin meterse en problemas

- El agente lee **automáticamente** el archivo `AGENTS.md` de la raíz del repo. Ahí dice qué archivos puede tocar cada uno y que no puede cambiar los `.hpp` ni los tests.
- Al empezar, péguenle el mensaje que está al final de su manual (`docs/MANUAL_<NOMBRE>.md`). Empieza con *"Soy …"* para que sepa quiénes son.
- Pídanle que **explique antes de escribir**. Cada uno graba su video en inglés defendiendo el código.
- Antes de aceptar un cambio, revisen que solo toque sus archivos. Antes de aprobar un comando, léanlo.
- La cuota gratuita del agente es **semanal**: no la gasten en cosas que el manual ya explica. El autocompletado (Tab) no tiene límite.

## Problemas típicos

| Síntoma | Solución |
|---|---|
| `"g++" no se reconoce` | PATH mal configurado (sección 2). Cierren y vuelvan a abrir Antigravity después de cambiar el PATH. |
| `build.bat` no se reconoce | Están en PowerShell: usen `.\build.bat`, o cambien la terminal a cmd (paso 4.5). |
| clangd marca errores en `#include "thermoplace/..."` | Abrieron una subcarpeta: abran la carpeta raíz `ThermoPlace` (donde está `compile_flags.txt`). |
| Los tests fallan con *cannot open benchmarks/...* | Corran los tests desde la raíz del repo, no desde `build\`. |
| `git push` rechazado (*permission denied*) | No aceptaron la invitación de colaborador, o iniciaron sesión con otra cuenta de GitHub. |
