---
name: Header/spec divergences from system prompt
description: The committed headers diverge from the system-prompt behavioral contract in several concrete ways. Confirm with user before implementing or "fixing" silently.
type: project
---

The system prompt describes a contract that the committed headers do not fully match. Before writing any .cpp implementation, confirm with the user which side wins.

Specific divergences observed in the headers (as of commit 7d02191):

1. **HashMap collision strategy.** System prompt says "separate chaining, 101 buckets". Header uses **open addressing with tombstones** (`Entry` struct has `occupied` and `isDeleted` flags, no linked-list `next`) and `initialCapacity = 16` with a `resize()` method. The DesignPhaseReport.md confirms open-addressing was an intentional design decision (section 5, "Design Changes", references the `isDeleted` tombstone).

2. **MinHeap backing storage.** System prompt says "backed by `std::vector` — no raw arrays". Header declares `Bill* heap` (raw pointer with manual `capacity`/`size`), explicitly a raw array.

3. **BudgetManager API shape.** System prompt specifies `logExpense()` returns `"ok" | "warning" | "exceeded"`, `getNextBill()`, `getExpensesByDateRange()`. Header has `addExpense()` (void), `checkBudget()` (void), `getExpensesByRange()`, plus a `hasPendingBills()` guard the prompt does not mention. There is no string-returning logExpense.

4. **BST.** System prompt says "raw pointer nodes" — header matches (`BSTNode* left/right`, custom `destroyTree`). This one is consistent.

**Why:** These divergences are large enough that picking the wrong side would mean rewriting completed work or contradicting the design report the user already submitted.

**How to apply:** Whenever the user asks for an implementation of HashMap, MinHeap, or BudgetManager, surface the divergence and ask which contract to honor. Default assumption: the committed header + DesignPhaseReport.md is the source of truth (it's been formally submitted), and the system prompt is an older or generic spec. But verify before writing code.
