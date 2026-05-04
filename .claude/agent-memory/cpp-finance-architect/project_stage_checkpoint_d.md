---
name: Project stage at Checkpoint D
description: Wave 4 done -- full ImGui UI with five fixed panels wired to BudgetManager. App is feature-complete for the portfolio scope.
type: project
---

As of 2026-05-04, Checkpoint D is committed (SHA `e9fb1f6`). The app builds clean with `make` and produces `finance_app`.

What landed in this checkpoint:
- `BudgetManager::getBudgetSnapshot()` -- value-copy vector of all tracked categories. Non-const because `HashMap::get()` is non-const. Drives the Budget Overview table and the post-Log-Expense status line.
- `src/main.cpp` -- five fixed ImGui panels in a 1280x720 grid:
  - Budget Overview (x=0,   y=0,   420x720)
  - Log Expense    (x=420, y=0,   430x360)
  - Bills          (x=420, y=360, 430x360)
  - Add Category   (x=850, y=0,   430x360)
  - Expense History (x=850, y=360, 430x360)
  All windows use `NoMove | NoResize | NoCollapse` and are positioned with `SetNextWindowPos/Size(..., ImGuiCond_Always)`.
- Makefile: `APP_SRC` now filters out `src/tests.cpp` so the test runner's `main()` does not collide with the app's `main()`.

UI design notes worth preserving:
- The "Log Expense" status line is derived from `getBudgetSnapshot()` after `addExpense()`, not by capturing stdout from `checkBudget()`. Keeps the render path side-effect-free.
- "Mark as Paid" uses a hardcoded payment date `Date(4, 5, 2025)` -- there is no date-picker for the payment date yet. Reasonable to revisit if the user wants the current date instead.
- ImGui version vendored is 1.92.8-WIP; all of `ImGuiTableFlags_SizingFixedFit`, `ImGuiTableFlags_ScrollY`, and `TableSetupScrollFreeze` are available in this version.

How to apply: Wave 5 (polish/persistence/reflection.md) is the natural next step. Do not assume the user wants any of those without being asked.
