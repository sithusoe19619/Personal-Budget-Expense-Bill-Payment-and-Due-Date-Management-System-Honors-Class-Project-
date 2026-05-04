# Finance Manager

A personal budget, expense, and bill management desktop application written in C++17, using ImGui + GLFW + OpenGL3 for the GUI.

Built as an honors project for CSC 331H (Spring 2026) at BMCC.

## Features

- **Budget tracking** — set per-category spending limits; get real-time OK / WARNING / EXCEEDED alerts
- **Expense logging** — log expenses by category and date; stored in a BST for efficient date-range queries
- **Bill management** — add bills with due dates; a min-heap always surfaces the next due bill in O(1)
- **Expense history** — query any date range across all logged expenses
- **Live dashboard** — five ImGui panels update in real time as you interact

## Data Structures

| Structure | Role | Complexity |
|-----------|------|------------|
| `HashMap` | Category budget lookup and update | O(1) amortized |
| `MinHeap` | Next-due bill priority queue | O(log n) insert, O(1) peek |
| `BST`     | Date-sorted expense storage and range queries | O(log n) insert, O(log n + k) range |

All three are implemented from scratch — no STL containers used as their internal backing store.

- **HashMap** uses open-addressing with linear probing and tombstone deletion. Load factor is capped at 0.7; exceeding it triggers a doubling resize with full rehash.
- **MinHeap** is backed by a raw `Bill*` array with manual capacity doubling. Ordering is by `dueDate` using the `Date::isBefore()` comparison.
- **BST** uses raw pointer nodes. Equal-date expenses go right so no expense is silently dropped. The `rangeQuery()` prunes entire subtrees for efficiency.

## Setup

### Prerequisites

```bash
xcode-select --install       # clang++ (Apple clang 16+)
brew install glfw pkg-config
```

### Download ImGui

Clone ImGui and copy the required files into the project:

```bash
git clone --depth=1 https://github.com/ocornut/imgui.git /tmp/imgui_src
mkdir -p imgui/backends
cp /tmp/imgui_src/imgui*.{h,cpp} imgui/
cp /tmp/imgui_src/imconfig.h /tmp/imgui_src/imstb_*.h imgui/
cp /tmp/imgui_src/backends/imgui_impl_glfw.{h,cpp} imgui/backends/
cp /tmp/imgui_src/backends/imgui_impl_opengl3.{h,cpp} imgui/backends/
cp /tmp/imgui_src/backends/imgui_impl_opengl3_loader.h imgui/backends/
```

### Build and run

```bash
make
./finance_app
```

## Testing

```bash
make test
```

Runs a standalone test suite (no GUI required) covering all three data structures and BudgetManager:

- `testDate` — comparison operators, isBetween, toString zero-padding
- `testCategoryInfo` — addExpense, getRemainingBudget, isOverBudget
- `testHashMap` — insert, get, contains, remove (tombstone), resize under load
- `testMinHeap` — insertion order, peek/extractMin ordering, markPaidByName
- `testBST` — inOrder sorted output, rangeQuery pruning, duplicate-date handling
- `testBudgetManager` — end-to-end: expenses, bills, range queries, bill payment

## Project Structure

```
finance-manager/
├── include/          # Header files (.h)
├── src/              # Implementation files (.cpp)
├── imgui/            # Vendored Dear ImGui source
│   └── backends/
├── Makefile
├── compile_flags.txt # clangd IDE support
└── reflection.md
```
