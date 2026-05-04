---
name: Project stage as of 2026-05-04
description: Snapshot of where the finance-manager project stands at audit time. Verify with git log/file reads before relying on this — implementation moves fast.
type: project
---

Audit snapshot taken 2026-05-04. State at commit 7d02191:

- All 8 headers in `include/` are complete and well-formed (Date, Expense, Bill, CategoryInfo, HashMap, MinHeap, BST, BudgetManager).
- All 8 files in `src/` are 1-line stubs containing only `#include "X.h"`. **Zero implementation logic exists.**
- No `main.cpp`, no `ui.cpp`, no `imgui/` directory, no GLFW/OpenGL linkage.
- `Makefile` uses `g++` (not `clang++` as system prompt assumes), target name is `finance-manager` (system prompt says `finance_app`), no ImGui/GLFW/OpenGL flags, no `-framework` lines for macOS.
- Git working tree has uncommitted deletes of `DesignPhase_Presentation.pptx` and `generate_slides.py`.
- Recent commits show the work so far has been Design Phase deliverables (UML, DesignPhaseReport.md, reflection.md, presentation), not implementation.

**Why:** The user is at the transition point from design to build phase. Knowing nothing is implemented yet means every "explain this code" question is actually "help me plan/write this from scratch."

**How to apply:** Before suggesting changes, re-verify with `wc -l src/*.cpp` and `ls include/` — once implementation starts, this snapshot ages out fast. Do not assume any data structure "works" until its `.cpp` has real code in it.
