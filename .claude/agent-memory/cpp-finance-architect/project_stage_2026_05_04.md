---
name: Project stage as of Wave 1 / Checkpoint A
description: Snapshot of finance-manager state after Wave 1 build setup. Verify with git log before relying — implementation moves fast.
type: project
---

Last update 2026-05-04 (Checkpoint A, commit 4fa37cb).

Wave 1 (build/scaffolding) is complete:

- ImGui vendored at `imgui/` (with `imgui_demo.cpp` and `backends/imgui_impl_opengl3_loader.h` included beyond the original spec — required for `ShowDemoWindow` to link and the OpenGL3 backend to load symbols).
- `Makefile` rewritten to clang++ / C++17 with `-DGL_SILENCE_DEPRECATION`, GLFW via `pkg-config`, Mac frameworks (Cocoa, OpenGL, IOKit, CoreVideo). Output binary is `finance_app` (note the underscore, not hyphen). Has a `test:` target referencing `src/tests.cpp` that does **not yet exist** — running `make test` will fail until that file is written.
- `src/main.cpp` exists with a minimal blank ImGui window (1280x720, OpenGL 3.3 core + forward-compat, vsync on, GLFW error callback). It calls `ImGui::ShowDemoWindow()` as a placeholder pending Wave 2 panels.
- Headers in `include/` (8 files) unchanged from design phase. Implementations in `src/` (8 .cpp files) are still 1-line stubs containing only `#include "X.h"` — **no real logic exists yet**.
- `.gitignore` added covering `finance_app`, `run_tests`, `.DS_Store`, `*.o`, `*.dSYM/`, editor cruft.

Build environment confirmed working on this Mac M3 / macOS 15.7.3:
- Homebrew installed `glfw 3.4` and `pkgconf 2.5.1` at `/opt/homebrew/`.
- `pkg-config --cflags glfw3` returns `-I/opt/homebrew/Cellar/glfw/3.4/include`.
- Build is warning-free with `-Wall -Wextra`.

**Why:** Knowing the build pipeline works lets future-you skip rediscovery. The next wave needs to flesh out `src/*.cpp` implementations and replace `ShowDemoWindow()` in `main.cpp` with the three real panels (Budget Overview, Log Expense form, Bills).

**How to apply:** Before assuming "the build works" later, run `make clean && make` once to confirm. The header-vs-spec divergences (separate file) still apply — confirm the contract before writing any data-structure implementation.
