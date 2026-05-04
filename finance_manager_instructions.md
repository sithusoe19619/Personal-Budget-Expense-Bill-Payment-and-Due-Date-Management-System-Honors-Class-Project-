# 💰 Personal Finance Manager — C++ Project Instructions

**Tech Stack:** C++17 · clang++ · ImGui + GLFW + OpenGL3 · Makefile · Mac M3

---

## Phase 1: System Design with UML + AI Support
⏰ ~45 mins

In this phase, you'll design your system before writing any code.

### Step 1: Understand the Problem

- [ ] Identify the three core user actions your app must support:
  - Log an expense and check if it exceeds a category budget
  - View the next upcoming bill and mark it as paid
  - Query spending history by date range
- [ ] Create `reflection.md` in your project folder
- [ ] Document the three actions in natural language under a **System Design** section

---

### Step 2: List the Building Blocks

Brainstorm each component. For each, determine attributes and methods.

Your system has **7 components**:

| Component | Role |
|---|---|
| `Expense` | Stores amount, category, date, description |
| `Bill` | Stores name, amount, due date, paid status |
| `CategoryInfo` | Stores budget limit and current spending total |
| `HashMap` | O(1) category budget lookup and update |
| `MinHeap` | Always surfaces the next due bill in O(1) |
| `BST` | O(log n) expense storage sorted by date |
| `BudgetManager` | Main controller — integrates all three data structures |

- [ ] For each component, list its attributes and methods on paper or in a notes file

---

### Step 3: Draft Your UML with AI

- [ ] Open Claude Code and ask:
  > "I'm designing a C++ personal finance app with these 7 components: Expense, Bill, CategoryInfo, HashMap, MinHeap, BST, and BudgetManager. Generate a Mermaid.js class diagram showing attributes, key methods, and relationships between all of them."
- [ ] Paste the Mermaid code into https://mermaid.live to preview it
- [ ] Review relationships:
  - `BudgetManager` contains `HashMap`, `MinHeap`, and `BST`
  - `HashMap` maps `string → CategoryInfo`
  - `MinHeap` stores `Bill` objects ordered by due date
  - `BST` stores `Expense` objects ordered by date
- [ ] Keep this diagram — you'll revisit it in Phase 7

> 💡 Mermaid is a text-based diagramming tool. Paste the generated code at mermaid.live to see your class diagram rendered visually.

---

### Step 4: Translate UML into C++ Skeletons

- [ ] Create this folder structure:
```
finance-manager/
├── include/        ← .h header files go here
├── src/            ← .cpp implementation files go here
├── README.md
└── reflection.md
```
- [ ] Ask Claude:
  > "Based on the UML diagram we just created, generate skeleton .h header files for all 7 components in C++. Use #ifndef / #define / #endif include guards in each file. Use std::string for text fields and double for monetary values."
- [ ] Review the skeletons — example `Expense.h`:
```cpp
#ifndef EXPENSE_H
#define EXPENSE_H
#include <string>

class Expense {
public:
    std::string category;
    std::string description;
    std::string date;       // format: "YYYY-MM-DD"
    double amount;

    Expense(std::string category, std::string description,
            std::string date, double amount);

    bool operator<(const Expense& other) const;
};

#endif
```
- [ ] Create matching empty `.cpp` files in `src/` that include their headers
- [ ] Commit:
```bash
git add .
git commit -m "chore: add C++ class skeletons from UML"
```

---

### Step 5: Reflect and Refine

- [ ] Open `reflection.md` and answer **"1a. Initial design"**:
  - Why HashMap for budgets? *(O(1) lookup every time an expense is logged)*
  - Why MinHeap for bills? *(always surfaces earliest due date in O(1))*
  - Why BST for expenses? *(efficient date-range queries in O(log n + k))*
- [ ] Ask Claude to review your headers:
  > "Review these header files in include/. Are there any missing relationships, redundant fields, or potential design bottlenecks I should fix before implementing?"
- [ ] Document any changes in **"1b. Design changes"**

> ✅ **Checkpoint:** You've created a Mermaid UML diagram and matching C++ class skeletons. Your system's blueprint is complete and ready for implementation.

---

## Phase 2: Project Setup
⏰ ~30 mins

---

### Step 1: Install Required Tools

- [ ] Install Xcode Command Line Tools (includes `clang++`):
```bash
xcode-select --install
```
- [ ] Install Homebrew (if not already installed):
```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```
- [ ] Install GLFW:
```bash
brew install glfw
```
- [ ] Verify clang++ works:
```bash
clang++ --version
```

---

### Step 2: Download ImGui

- [ ] Go to https://github.com/ocornut/imgui → Code → Download ZIP
- [ ] Create an `imgui/` folder in your project root
- [ ] Copy into `imgui/`:
  - `imgui.h`, `imgui.cpp`
  - `imgui_draw.cpp`, `imgui_tables.cpp`, `imgui_widgets.cpp`
  - `imgui_internal.h`, `imconfig.h`, `imstb_rectpack.h`, `imstb_textedit.h`, `imstb_truetype.h`
- [ ] Copy into `imgui/backends/`:
  - `imgui_impl_glfw.h`, `imgui_impl_glfw.cpp`
  - `imgui_impl_opengl3.h`, `imgui_impl_opengl3.cpp`

---

### Step 3: Create Your Makefile

- [ ] Ask Claude:
  > "Generate a Makefile for a C++17 project on Mac M3 (Apple Silicon) using clang++, GLFW, and ImGui with the OpenGL3 backend. Source files are in src/, headers in include/, ImGui files in imgui/ with backends in imgui/backends/. Output binary: finance_app."
- [ ] Save the Makefile in your project root
- [ ] Test it compiles:
```bash
make
```

> 💡 If you see linker errors, ask Claude: "My Makefile gives this error on Mac M3 Apple Silicon: [paste error]. How do I fix the linker flags?"

---

### Step 4: Hello World Test

- [ ] Ask Claude:
  > "Generate a minimal src/main.cpp that opens an ImGui + GLFW + OpenGL3 window on Mac M3 titled 'Finance Manager'. Just an empty window — no widgets yet."
- [ ] Run:
```bash
make && ./finance_app
```
- [ ] Confirm a window opens before moving on

> ✅ **Checkpoint:** Your project compiles and opens an empty ImGui window. The build pipeline is confirmed working.

---

## Phase 3: Core Data Structures
⏰ ~90 mins

Implement all three data structures from scratch — one at a time, testing each before moving to the next.

---

### Step 1: Implement HashMap

- [ ] Open `include/HashMap.h` and `src/HashMap.cpp`
- [ ] Implement using **separate chaining** (array of linked lists):
  - `insert(key, value)` — add or update
  - `get(key)` — return the value
  - `contains(key)` — check if key exists
  - `remove(key)` — delete a key
  - `getAll()` — return all pairs (for budget report)
- [ ] Ask Claude:
  > "Implement a HashMap in C++ using separate chaining with a fixed bucket count of 101. Key is std::string, value is CategoryInfo. Show full .h and .cpp implementation."
- [ ] Quick test in `main.cpp`:
```cpp
HashMap map;
map.insert("Food", CategoryInfo(500.0));
std::cout << map.get("Food").budget_limit << std::endl;  // expect: 500
```

> 💡 Why 101? Prime numbers reduce hash collisions — keys distribute more evenly across buckets.

---

### Step 2: Implement MinHeap

- [ ] Open `include/MinHeap.h` and `src/MinHeap.cpp`
- [ ] Implement using **`std::vector` as the underlying array**:
  - `push(bill)` — insert and bubble up
  - `top()` — return earliest bill without removing
  - `pop()` — remove and return earliest bill
  - `isEmpty()` — check if empty
  - `size()` — return count
- [ ] Ask Claude:
  > "Implement a MinHeap in C++ using std::vector that stores Bill objects ordered by due date. YYYY-MM-DD string comparison works correctly for ISO dates. Include heapifyUp and heapifyDown."
- [ ] Quick test: insert 3 bills with different dates — `top()` should always return the earliest

---

### Step 3: Implement BST

- [ ] Open `include/BST.h` and `src/BST.cpp`
- [ ] Implement using **raw pointer nodes** ordered by expense date:
  - `insert(expense)` — add in sorted order
  - `inOrder()` — return all expenses sorted by date
  - `rangeQuery(startDate, endDate)` — return expenses within range
  - `isEmpty()` — check if empty
- [ ] Ask Claude:
  > "Implement a BST in C++ using raw pointer nodes where each node stores an Expense object ordered by YYYY-MM-DD date string. Include insert, inOrder traversal, range query, and a recursive destructor to prevent memory leaks."

> 💡 BST nodes use `new` — your destructor must `delete` every node recursively or you'll have memory leaks.

> ✅ **Checkpoint:** All three data structures compile and pass basic tests. You've built the engine of your finance app from scratch.

---

## Phase 4: Application Logic
⏰ ~60 mins

---

### Step 1: Implement Supporting Classes

- [ ] Implement `Expense` fully: constructor + `operator<` for BST date comparison
- [ ] Implement `Bill` fully: constructor + `operator<` for MinHeap date comparison + `markPaid()`
- [ ] Implement `CategoryInfo`: `budget_limit`, `amount_spent`, `addExpense(amount)`, `percentUsed()`

---

### Step 2: Implement BudgetManager

- [ ] Implement all methods in `src/BudgetManager.cpp`:
  - `addCategory(name, budget_limit)` → inserts into HashMap
  - `logExpense(category, description, date, amount)` → updates HashMap, inserts into BST, returns `"ok"` / `"warning"` / `"exceeded"`
  - `addBill(name, amount, dueDate)` → pushes into MinHeap
  - `getNextBill()` → returns `MinHeap.top()`
  - `payNextBill()` → calls `MinHeap.pop()`
  - `getExpensesByDateRange(start, end)` → calls `BST.rangeQuery()`
  - `getBudgetReport()` → calls `HashMap.getAll()`
- [ ] Ask Claude:
  > "Implement the BudgetManager class in C++ integrating HashMap (category budgets), MinHeap (bills by due date), and BST (expense history by date). Show logExpense returning 'ok', 'warning', or 'exceeded' based on budget percentage."

---

### Step 3: CLI Test Before UI

- [ ] Test all logic in the terminal before touching ImGui:
```cpp
BudgetManager manager;
manager.addCategory("Food", 500.0);
manager.logExpense("Food", "Groceries", "2025-04-01", 450.0);
manager.logExpense("Food", "Restaurant", "2025-04-03", 80.0); // expect: "exceeded"

manager.addBill("Rent", 1200.0, "2025-05-01");
manager.addBill("Internet", 60.0, "2025-04-15");
std::cout << manager.getNextBill().name << std::endl; // expect: "Internet"
```

> ✅ **Checkpoint:** BudgetManager correctly logs expenses, triggers budget alerts, and surfaces the nearest bill — verified in the terminal before touching the UI.

---

## Phase 5: UI Integration with ImGui
⏰ ~45 mins

### What is ImGui?

**ImGui (Immediate Mode GUI)** is a lightweight C++ UI library that lets you create visual interfaces with real buttons, input fields, tables, and windows — without building them from scratch.

Unlike game engines or heavy frameworks, ImGui is designed for exactly this: developer tools, data dashboards, and simple apps where you need a clean UI fast.

**How it works:**
- Every frame, you call functions like `ImGui::Button("Log Expense")` and ImGui draws them on screen
- If a button is clicked, the function returns `true` — that's when you run your logic
- There's no event system — it's just `if (ImGui::Button(...)) { do_something(); }`

**What you'll use:**
| ImGui Function | What it does |
|---|---|
| `ImGui::Begin / End` | Creates a named panel/window |
| `ImGui::InputText` | Text input field |
| `ImGui::InputDouble` | Number input field |
| `ImGui::Button` | Clickable button |
| `ImGui::BeginTable` | Creates a data table |
| `ImGui::TextColored` | Colored text (for alerts) |
| `ImGui::Separator` | Horizontal dividing line |

---

### Step 1: Plan Your Layout

Your app will have three panels:
1. **Budget Overview** — table of all categories with spending vs. limit
2. **Log Expense** — input form (category, amount, date, description)
3. **Bills** — next due bill + form to add new bills

- [ ] Sketch the layout on paper before coding it

---

### Step 2: Build the Budget Overview Panel

- [ ] Use `ImGui::BeginTable` to display categories:
  - Columns: Category | Budget | Spent | % Used | Status
  - 🟢 Green = under 80% | 🟡 Yellow = 80–100% | 🔴 Red = over 100%
- [ ] Ask Claude:
  > "Show me how to create an ImGui table in C++ with colored text in a Status column — green under 80%, yellow 80–100%, red over 100%."

---

### Step 3: Build the Expense Input Form

- [ ] Add these ImGui widgets:
  - `ImGui::InputText` — category, description, date
  - `ImGui::InputDouble` — amount
  - `ImGui::Button("Log Expense")` — submit
- [ ] On submit: call `manager.logExpense(...)` and display the returned alert
- [ ] Ask Claude:
  > "Show me an ImGui form in C++ with InputText fields, an InputDouble, and a submit button that calls a function when clicked."

---

### Step 4: Build the Bills Panel

- [ ] Display `manager.getNextBill()` prominently — name, amount, due date
- [ ] Add `"Mark as Paid"` button calling `manager.payNextBill()`
- [ ] Add input form for new bills (name, amount, due date)

---

### Step 5: Connect and Run

- [ ] Replace CLI test code in `main.cpp` with full ImGui layout
- [ ] Run:
```bash
make && ./finance_app
```
- [ ] Verify all three panels work and update in real time

> 💡 If panels are overlapping, ask Claude: "My ImGui panels are misaligned. Here is my layout code: [paste code]. How do I restructure it using child windows?"

> ✅ **Checkpoint:** The app opens with a visual UI. You can log expenses, see budget alerts update, and view/pay bills.

---

## Phase 6: Testing and Verification
⏰ ~30 mins

---

### Step 1: Plan What to Test

- [ ] List core behaviors to verify:
  - HashMap — insert, get, update, missing key
  - MinHeap — `top()` returns earliest date, `pop()` removes correctly
  - BST — `rangeQuery()` returns correct subset, `inOrder()` is sorted
  - BudgetManager — alert thresholds trigger at correct percentages

---

### Step 2: Write Test Cases

- [ ] Create `src/tests.cpp` using `assert()`:
```cpp
#include <cassert>
#include <iostream>
#include "../include/HashMap.h"

void testHashMap() {
    HashMap map;
    map.insert("Food", CategoryInfo(500.0));
    assert(map.contains("Food") == true);
    assert(map.get("Food").budget_limit == 500.0);
    map.get("Food").addExpense(300.0);
    assert(map.get("Food").amount_spent == 300.0);
    std::cout << "HashMap: all tests passed\n";
}

int main() {
    testHashMap();
    // add testMinHeap(), testBST(), testBudgetManager()
    return 0;
}
```
- [ ] Write at least one test function per data structure
- [ ] Ask Claude:
  > "What are the most important edge cases to test for a MinHeap ordering bills by YYYY-MM-DD date strings, and a BST doing date range queries in C++?"

---

### Step 3: Run and Debug

- [ ] Add a `test` target to your Makefile:
```makefile
test:
	clang++ -std=c++17 -I./include src/tests.cpp src/HashMap.cpp \
	src/MinHeap.cpp src/BST.cpp src/BudgetManager.cpp \
	src/Expense.cpp src/Bill.cpp src/CategoryInfo.cpp \
	-o run_tests && ./run_tests
```
- [ ] Run:
```bash
make test
```
- [ ] Fix any failures — if stuck, ask Claude:
  > "This BST range query test is failing. Here is the test and my rangeQuery implementation: [paste code]. What is wrong?"

---

### Step 4: Finalize and Commit

- [ ] Add a **"Testing"** section to `README.md` with `make test` and what it covers
- [ ] Commit:
```bash
git add .
git commit -m "test: add data structure and budget manager test suite"
git push origin main
```

> ✅ **Checkpoint:** All tests pass. Core logic is verified independently of the UI.

---

## Phase 7: Polish, Documentation, and Reflection
⏰ ~30 mins

---

### Step 1: Polish the UI

- [ ] Ensure all three panels are clearly labeled and readable
- [ ] Add `ImGui::TextColored` budget alert messages (red for exceeded, yellow for warning)
- [ ] Take a screenshot of your running app for the README

---

### Step 2: Finalize Your UML

- [ ] Revisit the Mermaid diagram from Phase 1
- [ ] Ask Claude:
  > "Based on my final C++ implementation in include/, what updates should I make to my UML diagram to accurately reflect how my classes interact and what methods actually exist?"
- [ ] Update the Mermaid code to match the final implementation
- [ ] Save the rendered diagram as `uml_final.png` in your project folder

---

### Step 3: Polish Your README

- [ ] Project title and 2-sentence description
- [ ] Setup instructions — `brew install glfw`, download ImGui, `make`
- [ ] How to run — `./finance_app`
- [ ] Features list — budget tracking, bill prioritization, expense history queries
- [ ] Data Structures section — brief explanation of why each was chosen
- [ ] Testing section — `make test` and what it covers
- [ ] Demo screenshot

---

### Step 4: Write Your Reflection

- [ ] Open `reflection.md` and complete all sections:
  - **1a. Initial design** — the 7 components and their responsibilities
  - **1b. Design changes** — what changed during implementation and why
  - **2a. Data structure tradeoffs** — why HashMap over BST for budgets? Why MinHeap over sorted vector for bills?
  - **2b. Algorithm notes** — how your BST range query traverses the tree
  - **3. AI Strategy** — which Claude features were most useful, one suggestion you rejected or modified, what you learned about being the lead architect

- [ ] Final commit and push:
```bash
git add .
git commit -m "docs: finalize README, UML, and reflection"
git push origin main
```

> ✅ **Checkpoint:** Your project is fully documented, tested, and polished — a professional artifact that clearly tells the story from design to implementation.

---

## Optional Extensions
⏰ ~30 mins each

- [ ] **Extension 1: File Persistence** — Save and load all data between runs using a text file. Ask Claude: "How do I serialize a BST of Expense objects and a MinHeap of Bill objects to a plain text file in C++ and reload them on startup?"

- [ ] **Extension 2: Recurring Bills** — When a bill is paid, automatically add the next occurrence. Ask Claude: "How do I add one month to a YYYY-MM-DD date string in C++ without external libraries?"

- [ ] **Extension 3: Undo Last Expense** — Add a `std::stack<Expense>` to support undoing the last logged expense — adds a 4th data structure.

- [ ] **Extension 4: Export to CSV** — Add an "Export" button that writes all BST expenses to a `.csv` file. Ask Claude: "Show me how to write a CSV file in C++ from a vector of Expense objects."

- [ ] **Extension 5: Spending Chart** — Use `ImGui::PlotHistogram` to visualize monthly spending per category as a bar chart.

> ✅ **Checkpoint:** You've gone beyond the base requirements and built a more capable, professional finance tool.
