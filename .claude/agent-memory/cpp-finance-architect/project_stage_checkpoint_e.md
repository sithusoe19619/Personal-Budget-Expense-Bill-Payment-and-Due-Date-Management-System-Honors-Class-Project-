---
name: Project stage at Checkpoint E
description: Phase 7 done — README, reflection, and UML diagram finalized; project shippable.
type: project
---

Checkpoint E (commit `abb367b`, 2026-05-04) marks completion of Phase 7 — final documentation pass.

**Why:** Portfolio project required README setup steps, reflection covering tradeoffs/algorithm notes/AI-strategy, and an accurate UML diagram aligned with the final implementation. Without this checkpoint the project would compile and run but lack the artifacts a reviewer reads first.

**How to apply:**
- README now documents: feature list, data structure complexity table, ImGui vendor copy steps, `make` and `make test` invocations, project tree.
- reflection.md preserves the original Action 1/2/3 narrative + 1a/1b header-review changes + Traceability Matrix, then appends new sections: 1b Implementation-Phase Design Changes (s_categoryNames workaround, getBudgetSnapshot addition, Makefile rebuild), 2a Tradeoffs, 2b BST range-query algorithm note, 3 AI Strategy.
- diagram.mmd now shows: Date as `<<struct>>`, HashMap with Entry private struct, MinHeap with `Bill* heap` raw array, BST with BSTNode and destroyTree, BudgetManager with all final methods including `getBudgetSnapshot()` and `hasPendingBills()`. No `getAll()` on HashMap (matches reality).
- Push to origin/main succeeded. Remote moved to a renamed GitHub URL — local origin still works via redirect, but `git remote set-url` is the clean follow-up if the user cares.
