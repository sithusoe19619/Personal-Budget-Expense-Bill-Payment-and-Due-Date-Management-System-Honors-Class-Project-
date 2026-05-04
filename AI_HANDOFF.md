# AI Agent Handoff Document — Finance Manager

**Project:** Personal Finance Manager (C++ Honors Project)
**Course:** CSC 331H — Spring 2026, BMCC
**Author:** Sithu Soe (sithusoe19619@gmail.com)
**Last updated:** 2026-05-04
**Current status:** Feature-complete, fully tested, documented, and committed to `main`.

This document is written for any AI assistant (Copilot, Gemini, Codex, Claude, etc.) picking up this project. Read it fully before touching any code.

---

## 1. What This Project Is

A desktop personal finance manager built in **C++17** with an **ImGui + GLFW + OpenGL3** GUI. It runs as a native Mac app. The core purpose is to demonstrate three hand-built data structures (HashMap, MinHeap, BST) that power real-time budget tracking, bill prioritization, and expense history queries.

This is a **portfolio/academic project** — not a production app. Design decisions favor clarity and academic rigor over enterprise scale.

---

## 2. Tech Stack

| Layer | Technology | Version / Notes |
|---|---|---|
| Language | C++17 | clang++ (Apple Clang 16+) |
| GUI | Dear ImGui | Vendored in `imgui/` — immediate-mode, no event system |
| Window/Input | GLFW | Installed via `brew install glfw` |
| Renderer | OpenGL 3 | `#version 150`, core profile, forward-compat |
| Build | GNU Make | `Makefile` at project root |
| Platform | macOS (Apple Silicon M3) | Darwin 24.6.0 |
| IDE Support | clangd | `compile_flags.txt` and `.clangd` at root |

**ImGui is vendored** (source files copied into `imgui/`). It is NOT a git submodule and NOT managed by a package manager. The README has exact copy instructions.

---

## 3. Directory Layout

```
finance-manager/
├── include/                  ← All 8 header files (.h)
│   ├── Date.h
│   ├── Expense.h
│   ├── Bill.h
│   ├── CategoryInfo.h
│   ├── HashMap.h             ← Custom open-addressing hash map
│   ├── MinHeap.h             ← Custom raw-array min-heap
│   ├── BST.h                 ← Custom raw-pointer BST
│   └── BudgetManager.h       ← Controller class
├── src/                      ← All 10 implementation files (.cpp)
│   ├── Date.cpp
│   ├── Expense.cpp
│   ├── Bill.cpp
│   ├── CategoryInfo.cpp
│   ├── HashMap.cpp
│   ├── MinHeap.cpp
│   ├── BST.cpp
│   ├── BudgetManager.cpp
│   ├── main.cpp              ← GUI entry point + CLI smoke test
│   └── tests.cpp             ← Standalone test suite (NO ImGui/GLFW)
├── imgui/                    ← Vendored Dear ImGui source
│   └── backends/
├── Makefile
├── compile_flags.txt         ← clangd IDE paths
├── .clangd                   ← clangd config
├── README.md                 ← Setup, build, and feature docs
├── reflection.md             ← Academic reflection (do not delete)
├── diagram.mmd               ← Final Mermaid UML class diagram
├── DesignPhaseReport.md      ← Original design document (submitted)
├── finance_manager_instructions.md  ← Original project spec/phases
├── finance_app               ← Compiled binary (gitignored when clean)
└── run_tests                 ← Compiled test binary (gitignored when clean)
```

---

## 4. How to Build and Run

### Prerequisites (macOS only)
```bash
xcode-select --install       # Apple clang++
brew install glfw pkg-config
```

### Vendor ImGui (one-time, if imgui/ is missing)
```bash
git clone --depth=1 https://github.com/ocornut/imgui.git /tmp/imgui_src
mkdir -p imgui/backends
cp /tmp/imgui_src/imgui*.{h,cpp} imgui/
cp /tmp/imgui_src/imconfig.h /tmp/imgui_src/imstb_*.h imgui/
cp /tmp/imgui_src/backends/imgui_impl_glfw.{h,cpp} imgui/backends/
cp /tmp/imgui_src/backends/imgui_impl_opengl3.{h,cpp} imgui/backends/
cp /tmp/imgui_src/backends/imgui_impl_opengl3_loader.h imgui/backends/
```

### Build the app
```bash
make
./finance_app
```

### Run tests (no GUI required)
```bash
make test
```
All 6 test functions must print `PASS: <name>` and end with `All tests passed.`

### Clean
```bash
make clean
```

---

## 5. Architecture Overview

### 5.1 Data Flow

```
User Input (ImGui form)
        │
        ▼
  BudgetManager          ← single controller instance in main.cpp
  ├── addExpense()  ──►  BST.insert()          (date-sorted tree)
  ├── addBill()     ──►  MinHeap.insert()       (due-date priority queue)
  └── setBudgetLimit() ► HashMap.insert()       (category → CategoryInfo)

Render loop (every frame):
  BudgetManager.getBudgetSnapshotForMonth()
     ├── HashMap.get()           (O(1) category lookup)
     ├── BST.rangeQuery()        (O(log n + k) per category per month)
     └── s_monthlyLimits[]       (std::map for per-month budget overrides)
```

### 5.2 The Three Custom Data Structures

#### HashMap (`include/HashMap.h`, `src/HashMap.cpp`)
- **Purpose:** Maps category name (string) → `CategoryInfo` (budget limit + total spent)
- **Strategy:** Open addressing with **linear probing** and **tombstone deletion**
- **Key details:**
  - Initial capacity: 16; doubles when load factor exceeds 0.7
  - Tombstone: `Entry.occupied=true, Entry.isDeleted=true` — probe chains stay intact after deletion
  - Hash function: djb2 (`h = h*33 + c`)
  - `get()` returns a **raw pointer** (`CategoryInfo*`) — **this pointer is invalidated if `insert()` triggers a resize**. Always re-fetch after any insert.
  - No iterator / `getAll()` method — BudgetManager works around this with a separate `static std::vector<std::string> s_categoryNames` in `BudgetManager.cpp`

#### MinHeap (`include/MinHeap.h`, `src/MinHeap.cpp`)
- **Purpose:** Priority queue of `Bill` objects, ordered by earliest due date
- **Strategy:** Raw `Bill*` array with manual capacity doubling
- **Key details:**
  - Ordering: `Date::isBefore()` comparison (year → month → day integer fields)
  - `peek()` / `extractMin()` both `assert(size > 0)` — caller must check `isEmpty()` first
  - `markPaidByName()`: O(n) linear scan, sets `Bill.isPaid = true` in place — does NOT re-heapify
  - `removeByName()`: O(n) scan, replaces element with last, decrements size, runs both heapifyUp and heapifyDown on the replacement
  - `getAllBills()`: returns a sorted `std::vector<Bill>` copy (does NOT drain the heap)

#### BST (`include/BST.h`, `src/BST.cpp`)
- **Purpose:** Stores all `Expense` objects sorted by date for efficient range queries
- **Strategy:** Raw pointer nodes (`BSTNode*`), recursive insert/traverse/destroy
- **Key details:**
  - **Duplicate dates go RIGHT** — this is intentional so no expense is silently dropped
  - `rangeQuery()` prunes entire subtrees: if node date < start, skip left; if node date > end, skip left — O(log n + k)
  - `search(date)` is implemented as `rangeQuery(date, date)` — returns all expenses on that exact date
  - No delete/rebalance — insert-only tree is sufficient for this use case
  - Destructor calls `destroyTree(root)` recursively — no memory leaks

### 5.3 BudgetManager (`include/BudgetManager.h`, `src/BudgetManager.cpp`)

The single controller class that owns all three data structures.

**Critical implementation detail — the static category registry:**
```cpp
// BudgetManager.cpp (file scope, NOT a class member)
static std::vector<std::string> s_categoryNames;
static std::map<std::string, double> s_monthlyLimits;
```
These are file-scope statics. This means:
- If you ever create two `BudgetManager` instances (e.g., in tests), their category lists will collide.
- The test suite in `tests.cpp` uses exactly ONE `BudgetManager` instance in `testBudgetManager()` for this reason.
- This is acceptable for the single-instance app scope.

**Per-month budget limits:**
- `setBudgetLimit(category, limit, month, year)` stores a monthly override in `s_monthlyLimits` keyed as `"category-YYYY-MM"`.
- `getLimitForMonth()` checks `s_monthlyLimits` first, falls back to `CategoryInfo::budgetLimit` if no override exists.
- `getSpentInMonth()` does a BST range query from day 1 to day 31 of the given month, then filters by category. Day 31 is a safe ceiling because the Date comparison is year → month → day, so month matching takes priority.

---

## 6. The ImGui UI (`src/main.cpp`)

### 6.1 Layout — 5 Fixed Panels

The window is divided into a 3-column layout that scales dynamically with window size:

```
┌────────────────────┬──────────────┬──────────────┐
│                    │  Set Budget  │  Log Expense │
│  Budget Overview   │  (mid-top)   │  (right-top) │
│  (left, full H)    ├──────────────┼──────────────┤
│                    │    Bills     │  Expense     │
│                    │  (mid-bot)   │  History     │
│                    │              │  (right-bot) │
└────────────────────┴──────────────┴──────────────┘
```

Column widths:
- Column 1 (Budget Overview): 33% of window width
- Columns 2 & 3: split the remaining 67% equally

### 6.2 Panels

| Panel | Position | Key behavior |
|---|---|---|
| **Budget Overview** | Left, full height | Month navigator (← →); calls `getBudgetSnapshotForMonth()`; color-coded status (green/yellow/red) |
| **Set Budget** | Middle top | `InputText` + `InputDouble` + month/year int inputs; calls `setBudgetLimit(name, limit, month, year)` |
| **Bills** | Middle bottom | Table of all pending bills via `getAllBills()`; per-row "Mark as Paid" button; add-bill form |
| **Log Expense** | Right top | Category, description, amount, date inputs; calls `addExpense()`; shows inline OK/WARNING/EXCEEDED feedback |
| **Expense History** | Right bottom | Scrollable table of ALL expenses via `getExpensesByRange(Date(1,1,2000), Date(31,12,2099))` |

### 6.3 Scaling
```cpp
float scale = min(W/1280.0f, H/720.0f);  // clamped [0.5, 4.0]
io.FontGlobalScale = scale;
ImGui::GetStyle().ScaleAllSizes(scale);
```
The base style is saved once at startup as `baseStyle` and re-applied every frame before scaling, so it never compounds.

### 6.4 Fullscreen Toggle
- **F11**: toggles fullscreen (saves/restores window position and size)
- **ESC**: exits fullscreen without closing the window

### 6.5 Known Bug in Bills Panel
The "Mark as Paid" handler hardcodes the payment date:
```cpp
// src/main.cpp:311
manager.removeBill(toRemove, Date(4, 5, 2025));  // ← hardcoded date
```
This should use `Date(todayDay, todayMonth, todayYear)` instead. It is a known issue left for future cleanup.

---

## 7. Key Contracts and Invariants

These are non-obvious rules that caused bugs in the past. Violating them will break things silently.

1. **Never call `MinHeap::peek()` or `extractMin()` on an empty heap.** Both `assert(size > 0)`. Always call `isEmpty()` first (or `BudgetManager::hasPendingBills()`).

2. **`HashMap::get()` returns a pointer that is invalidated by the next `insert()` if it triggers a resize.** Pattern to follow:
   ```cpp
   // WRONG — pointer may dangle after addExpense inserts a new category
   CategoryInfo* p = categoryMap.get("Food");
   addExpense(someExpense);  // may resize HashMap
   p->totalSpent;            // use-after-free possible
   
   // CORRECT
   addExpense(someExpense);
   CategoryInfo* p = categoryMap.get("Food");  // re-fetch after insert
   ```

3. **BST duplicate dates go RIGHT.** This is intentional. Any code that walks the BST must not assume unique dates per node.

4. **`s_categoryNames` in `BudgetManager.cpp` is file-scope static.** Only one `BudgetManager` instance should exist per process. Creating two (e.g., in tests) causes category name leakage between them.

5. **`tests.cpp` has its own `main()`.** It cannot be compiled together with `main.cpp`. The Makefile `APP_SRC` filters it out with `$(filter-out src/tests.cpp,$(wildcard src/*.cpp))`. Do not remove this filter.

6. **The `Date` struct stores `(day, month, year)` as separate integers** — NOT a string. All comparisons are integer-based (year first, then month, then day). There is no string date anywhere in the codebase.

---

## 8. Completed Phases (What Has Already Been Done)

| Phase | Description | Status |
|---|---|---|
| Phase 1 | System design, UML diagram, C++ class skeletons | Done |
| Phase 2 | Build environment: GLFW, ImGui vendored, Makefile, blank window | Done |
| Phase 3 | HashMap, MinHeap, BST implemented from scratch | Done |
| Phase 4 | BudgetManager implemented; CLI smoke test passes | Done |
| Phase 5 | Full ImGui UI: 5 panels, dynamic layout, F11 fullscreen | Done |
| Phase 6 | Full test suite (`make test`): 6 functions, all passing | Done |
| Phase 7 | README, reflection.md, diagram.mmd finalized | Done |
| Post-7 | Per-month budget limits (month/year input on Set Budget panel) | Done |
| Post-7 | Date inputs default to today's real clock date | Done |
| Post-7 | Dynamic panel scaling with window resize | Done |

The project is **academically complete**. Everything past this point is extension work.

---

## 9. What Has NOT Been Done (Open Work)

### 9.1 Quick Bug Fixes
- Bills panel "Mark as Paid" uses a hardcoded date `Date(4, 5, 2025)` — should use today's date (`Date(todayDay, todayMonth, todayYear)`)
- Budget Overview month navigation bounds are frozen at the startup month — the `maxMonth`/`maxYear` are `const` and don't advance as time passes within a session

### 9.2 Officially Planned Extensions (from `finance_manager_instructions.md`)
These were listed in the original project spec as optional:

| Extension | Effort | Description |
|---|---|---|
| File Persistence | Medium | Serialize BST + MinHeap to plain text; reload on startup |
| Recurring Bills | Low | Auto-add next occurrence when a bill is marked paid |
| Undo Last Expense | Low | `std::stack<Expense>` — would add a 4th data structure |
| Export to CSV | Low | "Export" button writes all expenses to `.csv` |
| Spending Chart | Medium | `ImGui::PlotHistogram` for monthly per-category bar chart |

### 9.3 AI Expansion (Future — AWS Bedrock Backend)
Planned AI features requiring `libcurl` + AWS Signature V4 + `nlohmann/json`:

| Feature | Priority | What it does |
|---|---|---|
| Finance Chat Assistant | High | Natural language Q&A over the user's own data |
| Natural Language Expense Entry | High | "spent $12 on lunch today" → parsed expense entry |
| Spending Insights Panel | Medium | Proactive trend/anomaly analysis |
| Category Auto-Suggest | Medium | AI suggests category from description |
| Monthly Summary Report | Medium | Natural language monthly recap |
| Budget Recommendation Engine | Low | Suggests realistic limits from spending history |
| Spending Forecast | Low | Predicts next month's expenses |
| Bill Priority Advisor | Low | Ranks which bills to pay first given balance |

**Implementation path for Bedrock features:**
1. Add `libcurl` dependency: `brew install curl`
2. Implement AWS SigV4 signing (header: `Authorization: AWS4-HMAC-SHA256 ...`)
3. Add `nlohmann/json` for JSON parsing (single-header library)
4. Create a new ImGui panel for each feature
5. Wire panel inputs → Bedrock API call → parse response → display in ImGui

---

## 10. File-by-File Reference

### `src/Date.cpp`
Implements `Date` struct methods. All comparisons are integer-based: year first, then month, then day. `toString()` uses `snprintf` with zero-padding (`%04d-%02d-%02d`).

### `src/Expense.cpp` / `src/Bill.cpp` / `src/CategoryInfo.cpp`
Thin value objects. `Bill::markAsPaid(date)` sets `isPaid = true` and `paidOn = date`. `CategoryInfo::addExpense(amount)` increments `totalSpent`.

### `src/HashMap.cpp`
djb2 hash. Open addressing with linear probing. Load factor guard at 0.7 triggers doubling resize. Tombstone deletion preserves probe chains. See Section 5.2 for full details.

### `src/MinHeap.cpp`
Raw `Bill*` array. `heapifyUp` on insert, `heapifyDown` on extract. `removeByName` runs BOTH heapifyUp and heapifyDown on the replacement element (necessary because you don't know which direction the tree needs to fix). See Section 5.2.

### `src/BST.cpp`
Recursive insert/traverse/destroy. Equal dates go right. `rangeHelper` prunes subtrees. `search(d)` delegates to `rangeQuery(d, d)`. See Section 5.2.

### `src/BudgetManager.cpp`
- File-scope `s_categoryNames` (vector of strings) — the only way to enumerate all categories since HashMap has no iterator
- File-scope `s_monthlyLimits` (std::map keyed `"category-YYYY-MM"`) — per-month budget overrides
- `getSpentInMonth()`: BST range query then filter by category — O(log n + k) where k = expenses in the month
- `getBudgetSnapshotForMonth()`: builds a vector of `CategoryInfo` copies with `totalSpent` and `budgetLimit` computed for the requested month

### `src/main.cpp`
- Runs CLI smoke test first (prints to stdout before the window opens)
- Saves `baseStyle` once at startup, re-scales it every frame
- Layout: three column widths computed as fractions of window size each frame
- `todayDay/Month/Year` captured once at startup from `std::time(nullptr)` + `std::localtime()`
- All state (form input buffers, view month, etc.) is local to `main()`

### `src/tests.cpp`
Standalone test binary. Uses `assert()` — a failing assert aborts with file/line. No ImGui/GLFW. Run with `make test`. The six test functions cover: Date, CategoryInfo, HashMap (including resize stress test), MinHeap, BST (including duplicate dates and range pruning), and BudgetManager.

---

## 11. How to Add a New Feature

### Adding a new ImGui panel
1. Decide its grid position: `SetNextWindowPos(ImVec2(x, y))` and `SetNextWindowSize(ImVec2(w, h))`.
2. Compute `x`, `y`, `w`, `h` as fractions of `W` and `H` (window dimensions fetched each frame via `glfwGetWindowSize`).
3. Add the `BudgetManager` method it needs (if any) — implement in `.cpp` and declare in `.h`.
4. Open the panel with `ImGui::Begin("Panel Title", nullptr, fixedFlags)` and close with `ImGui::End()`.
5. Use `ImGui::BeginTable` for any tabular data.

### Adding a new BudgetManager method
1. Declare in `include/BudgetManager.h`.
2. Implement in `src/BudgetManager.cpp`.
3. If the method needs to iterate all categories, use `s_categoryNames` (already available at file scope in `BudgetManager.cpp`).
4. Add a test case in `src/tests.cpp` inside `testBudgetManager()` (or a new test function).

### Adding a new data structure
1. Create `include/NewDS.h` and `src/NewDS.cpp`.
2. Add the source file to the `test` target in the `Makefile` (it is already covered by `$(wildcard src/*.cpp)` in `APP_SRC`, minus `tests.cpp`).
3. Write test functions in `src/tests.cpp` and call them from `main()`.

### Adding file persistence
Suggested approach for the BST:
```cpp
// Save: inOrder() gives a sorted vector — write one expense per line
// Load: read line by line, parse fields, call insert() for each
```
Suggested format per line:
```
YYYY-MM-DD,category,amount,description
```
For MinHeap: `getAllBills()` returns a vector — write one bill per line.

---

## 12. Common Mistakes to Avoid

1. **Do NOT add `src/tests.cpp` to `APP_SRC`.** It has its own `main()`. The Makefile already filters it out.

2. **Do NOT call `ImGui::SetNextWindowPos/Size` with absolute pixel coordinates.** Always derive from `W` and `H` (the runtime window dimensions), otherwise the layout breaks on window resize or fullscreen.

3. **Do NOT use `std::vector` as the backing store for HashMap or MinHeap.** Both use raw arrays (`new[]`/`delete[]`). This was an intentional design decision documented in `DesignPhaseReport.md`.

4. **Do NOT add a `getAll()` iterator to HashMap.** The current workaround (`s_categoryNames` in `BudgetManager.cpp`) is intentional — changing the HashMap contract would break the documented design.

5. **Do NOT create more than one `BudgetManager` instance in any single binary.** The static category registry is not instance-isolated.

6. **Do NOT use string dates anywhere.** The `Date` struct uses integer fields. All comparison is integer arithmetic.

---

## 13. Build Troubleshooting

| Symptom | Likely cause | Fix |
|---|---|---|
| `pkg-config: not found` | pkg-config not installed | `brew install pkg-config` |
| `GLFW/glfw3.h: No such file` | GLFW not installed | `brew install glfw` |
| `imgui.h: No such file` | ImGui not vendored | Follow Section 4 vendor steps |
| `duplicate symbol _main` | `tests.cpp` included in app build | Verify Makefile `APP_SRC` filter-out |
| `Assertion failed: size > 0` | `peek()`/`extractMin()` on empty heap | Add `hasPendingBills()` guard |
| Pointer crash after budget set | Stale `CategoryInfo*` after resize | Re-fetch pointer after any `insert()` call |
| clangd shows wrong includes | `compile_flags.txt` paths are absolute | Update paths in `compile_flags.txt` to match new machine |

---

## 14. Git History Summary

```
25a6c14  feat: per-month budget limits with explicit month/year input
18e2568  fix: default expense and bill date inputs to today's real-clock date
1718b6b  feat: monthly budget periods with BST-based spending computation
3419c18  feat: scale fonts and widgets dynamically with window size
7d55aad  feat: dynamic panel layout + F11 fullscreen toggle
3f814e3  ui: rename Bills button to "Mark as Paid"
96d1846  feat: Bills panel shows all pending bills with per-row Mark as Paid
9e13c5a  fix: Mark as Paid now removes bill from heap via payNextBill()
abb367b  docs: Phase 7 — README, reflection, and UML diagram finalized
e9fb1f6  feat: Phase 5 -- ImGui UI with Budget Overview, Log Expense, Bills...
88bbe18  test: Phase 6 — full test suite for Date, HashMap, MinHeap, BST...
1fcbc9d  feat: Phase 4 — BudgetManager implemented, CLI smoke test passes
```

---

## 15. Quick Orientation Checklist for a New AI Agent

Before writing any code:

- [ ] Read this document fully
- [ ] Read `README.md` — setup and feature overview
- [ ] Read `include/BudgetManager.h` — the full public API
- [ ] Read `src/BudgetManager.cpp` — understand `s_categoryNames` and `s_monthlyLimits`
- [ ] Read `src/main.cpp` — understand the layout grid and all panel connections
- [ ] Run `make test` and confirm all tests pass on the current machine
- [ ] Run `make && ./finance_app` and confirm the window opens

When the user asks for a new feature:
1. Identify which panel it affects (or if it's a new panel)
2. Identify which `BudgetManager` method(s) are needed
3. Check if the data structures need new methods (they usually don't)
4. Write the `BudgetManager` method first, test it in `tests.cpp`, then wire the UI

---

*This document was written on 2026-05-04 to ensure smooth handoff between AI tools.*
