---
name: Project stage at Checkpoint C
description: Wave 3 complete — BudgetManager wired up, CLI smoke test green. UI still ImGui demo window.
type: project
---

Checkpoint C committed at 1fcbc9d on 2026-05-04.

State of the codebase at this point:
- All data structures (HashMap, MinHeap, BST) and value types (Date, Expense, Bill, CategoryInfo) implemented in src/.
- BudgetManager fully implemented in src/BudgetManager.cpp.
- src/main.cpp runs a CLI smoke test before opening the GLFW window. Output verified: WARNING at 80%, EXCEEDED at 110%, OK at 25%, Internet returned as next bill, 2 expenses in Apr 1-4 range, report prints both categories with OVER/under status.
- ImGui main loop still shows ShowDemoWindow() — finance UI panels (Wave 4) not yet wired.

**Why:** Logging the integration milestone so a future session can pick up at Wave 4 (UI panels) without re-deriving what already works.

**How to apply:** If asked about next steps, the open work is the three ImGui panels (Budget Overview / Log Expense / Bills) plus replacing ShowDemoWindow() with them. Don't re-implement BudgetManager — read it first.

Notable implementation choices worth remembering:
- BudgetManager.cpp uses a file-scope `static std::vector<std::string> s_categoryNames` to compensate for HashMap having no iterator/getAll. Single-instance assumption documented in the file. If the project ever needs two BudgetManager objects, this is the breakpoint.
- generateReport() prints "OVER" / "under" / "no-limit" status per category.
- checkBudget() treats budgetLimit==0.0 as "no budget set" and short-circuits before doing percent math (avoids div-by-zero).
