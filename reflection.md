# Reflection

## System Design

This document describes the three core user actions that the Personal Budget, Expense, Bill Payment and Due Date Management System must support.

---

### Action 1: Log an Expense and Check if it Exceeds a Category Budget

The user selects a spending category (such as Food, Transportation, or Entertainment), enters an amount spent, a date, and an optional description. The system records this expense and immediately compares the running total for that category against the user-defined budget limit. If the total spending in that category now exceeds the budget, the system notifies the user with a warning showing how much over budget they are. This action allows users to stay aware of their spending habits in real time, category by category.

---

### Action 2: View the Next Upcoming Bill and Mark it as Paid

The user can request to see their next upcoming bill. The system scans all unpaid bills, finds the one with the nearest due date, and displays its name, due date, and amount owed. The user then has the option to mark that bill as paid. When marked paid, the system records the date of payment and updates the bill's status so it no longer appears as upcoming. This keeps users informed about what financial obligations are coming up and lets them track which bills have already been handled.

---

### Action 3: Query Spending History by Date Range

The user provides a start date and an end date. The system retrieves all expenses that were logged within that date range and displays them in order. The results show each expense's date, category, amount, and description, along with a subtotal per category and a grand total for the entire period. This action allows users to review their financial history, identify spending patterns, and understand where their money went over any given period of time.

---

## Component Brainstorm

The system is composed of five classes. Three are data-holding structs (Date, Expense, Bill) and two are manager classes that own collections and enforce business logic (BudgetManager, BillManager).

---

### Date

A reusable value type used by both Expense and Bill to represent calendar dates. Kept as its own component so date comparison and formatting logic lives in one place.

**Attributes:**
- `int day` — day of the month (1–31)
- `int month` — month of the year (1–12)
- `int year` — four-digit year

**Methods:**
- `bool isBefore(Date other)` — returns true if this date comes before another; used to find the next upcoming bill and to filter expenses by range
- `bool isAfter(Date other)` — returns true if this date comes after another
- `bool isBetween(Date start, Date end)` — returns true if this date falls within an inclusive range; used in the spending history query
- `string toString()` — formats the date as a readable string (e.g., "03/30/2026") for display

---

### Expense

Represents a single spending event recorded by the user. This is a plain data-holding struct with no business logic of its own.

**Attributes:**
- `Date date` — when the expense occurred
- `string category` — which budget category it belongs to (e.g., "Food", "Transport")
- `double amount` — how much was spent
- `string description` — optional note the user provides for context

**Methods:**
- `void display()` — prints a formatted one-line summary of the expense for use in history queries and receipts

---

### Bill

Represents a financial obligation with a due date. Tracks whether the bill has been paid and when.

**Attributes:**
- `string name` — label for the bill (e.g., "Electric Bill", "Rent")
- `Date dueDate` — when payment is due
- `double amountDue` — the amount owed
- `bool isPaid` — whether the bill has been settled
- `Date paidOn` — the date the user marked it as paid (only meaningful when isPaid is true)

**Methods:**
- `void markAsPaid(Date paymentDate)` — sets isPaid to true and records the paymentDate
- `void display()` — prints a formatted summary of the bill including its status and due date

---

### BudgetManager

The central manager for all expense-related operations. Owns the list of expenses and the per-category budget limits. Enforces the budget check on every new expense.

**Attributes:**
- `vector<Expense> expenses` — the full history of all logged expenses
- `map<string, double> budgetLimits` — maps each category name to its user-defined spending cap
- `map<string, double> categoryTotals` — tracks the running spend total per category, updated on every log

**Methods:**
- `void setBudget(string category, double limit)` — sets or updates the spending cap for a category
- `void logExpense(Expense e)` — adds the expense to the history, updates the running total for its category, and calls checkBudget
- `void checkBudget(string category)` — compares the category's running total against its limit and prints a warning if it is exceeded
- `double getCategoryTotal(string category)` — returns how much has been spent in a given category so far
- `vector<Expense> getExpensesByDateRange(Date start, Date end)` — filters and returns all expenses whose dates fall within the given range, used by Action 3

---

### BillManager

Owns and manages the list of bills. Responsible for finding the next due bill and processing payment.

**Attributes:**
- `vector<Bill> bills` — the complete list of all bills, paid and unpaid

**Methods:**
- `void addBill(Bill b)` — adds a new bill to the list
- `Bill* getNextUnpaidBill()` — scans all unpaid bills, finds the one with the earliest due date, and returns a pointer to it; returns nullptr if all bills are paid
- `void markBillAsPaid(string billName, Date paymentDate)` — finds the bill by name and calls its markAsPaid method
- `void displayAllBills()` — prints a summary of every bill with its status, for an overview screen

---

## Data Structure Justification

### 1a. Initial Design

**Why HashMap for budgets?**

Every time the user logs an expense, the system must immediately look up that expense's category, retrieve the running total, add to it, and compare it against the budget limit. This happens on every single log operation, so the lookup must be as fast as possible. A HashMap maps category names (strings) directly to their `CategoryInfo` objects using a hash function, giving O(1) average-case lookup regardless of how many categories exist. Any other structure — a sorted array, a linked list, or even a BST — would require O(log n) or O(n) to find the right category on every expense log. Since budget checking is the most frequent operation in the system, O(1) is the right choice here.

**Why MinHeap for bills?**

The most common bill operation is Action 2: find the next upcoming bill. A MinHeap keeps the bill with the earliest due date at the root at all times, so retrieving it is O(1) — no scanning required. When a bill is marked as paid and removed, the heap reorganizes itself in O(log n). This is a direct match for the use case: the system always needs the minimum (earliest) due date, which is exactly what a MinHeap is optimized for. A plain array or list would require scanning all unpaid bills every time — O(n) — which gets slower as the bill count grows.

**Why BST for expenses?**

Action 3 asks for all expenses between a start date and an end date. A BST stores expenses sorted by date, so the system can descend the tree to the start date in O(log n) and then collect every node in range by continuing the traversal — stopping as soon as it passes the end date. The total cost is O(log n + k), where k is the number of results returned. A flat array or vector could only do this in O(n) by scanning every expense. As spending history grows over months or years, the BST keeps range queries fast while the array approach degrades linearly.

---

### 1b. Design Changes

The following changes were made during header review before implementation began. Each change was driven by a correctness issue, an uninitialized field, or a missing operation discovered when tracing how the components interact.

---

**`Date` — added `operator==`, default constructor, parameterized constructor**

The initial design had no way to compare two dates for equality. `BST::search()` needs to know when it has found an exact date match, and `Bill` comparison requires it too. `operator==` was added to cover this. Two constructors were also added: a default constructor that zeroes all fields (preventing garbage `int` values), and a parameterized constructor `Date(int d, int m, int y)` so that dates can be created in a single line throughout the implementation.

---

**`Expense` — added default constructor**

`double amount` had no default value and would hold garbage on a freshly created object. A default constructor `Expense() : amount(0.0)` was added to ensure clean initialization.

---

**`Bill` — `markAsPaid` parameter changed to `const Date&`, constructor added**

`markAsPaid` originally took `Date` by value, which is inconsistent with how `Date` is passed everywhere else in the system. Changed to `const Date&`. A default constructor `Bill() : amountDue(0.0), isPaid(false)` was also added — without it, `isPaid` and `amountDue` would hold garbage values and any bill logic would be undefined behavior.

---

**`CategoryInfo` — added default constructor**

`budgetLimit` and `totalSpent` were uninitialized doubles. `getRemainingBudget()` and `isOverBudget()` would return garbage on any freshly created object. A constructor `CategoryInfo() : budgetLimit(0.0), totalSpent(0.0)` was added.

---

**`HashMap` — added `isDeleted` tombstone, `Entry` constructor, Rule of Three**

Three changes were made. First, the `Entry` struct was missing an `isDeleted` flag. Open-addressing deletion requires a tombstone marker so that probe chains are not broken when an entry is removed — without it, `get()` would stop searching too early and miss valid entries. Second, `Entry` had no constructor, so `occupied` and `isDeleted` would be uninitialized booleans when `new Entry[n]` is called. `Entry() : occupied(false), isDeleted(false)` was added. Third, `HashMap` owns a raw pointer `Entry* buckets` and has a destructor, so the copy constructor and copy assignment operator were deleted to prevent shallow copies and double-frees.

---

**`MinHeap` — added Rule of Three, `markPaidByName()`**

`MinHeap` owns `Bill* heap` and has a destructor, so the copy constructor and copy assignment operator were deleted. A `markPaidByName(const std::string& name, const Date& paymentDate)` method was also added. Without it, `BudgetManager::markBillPaid()` had no way to update a specific bill inside the heap — the only options available were `extractMin()`, `peek()`, and `isEmpty()`, none of which support finding a bill by name. `markPaidByName` searches the heap array linearly, updates `isPaid` and `paidOn` in place, and requires no re-heapify since the due date does not change.

---

**`BST` — added `BSTNode` constructor, `inOrderHelper`, explicit `Date.h` include, Rule of Three, duplicate date strategy, changed `search()` return type**

Several changes were made. `BSTNode` had no constructor, so `left` and `right` were uninitialized raw pointers — following them during traversal would be undefined behavior. A constructor `BSTNode(const Expense& e) : data(e), left(nullptr), right(nullptr)` was added. A private `inOrderHelper` was missing despite `inOrder()` needing to recurse. `Date.h` was added as an explicit include since `Date` appears in method signatures and relying on transitive inclusion is fragile. The Rule of Three was applied since `BST` owns a raw pointer `BSTNode* root`. A comment was added documenting the duplicate date strategy — equal dates go right — so all expenses on the same day are preserved rather than silently overwritten. Finally, `search()` was changed from `Expense*` to `std::vector<Expense>` to correctly handle multiple expenses on the same date, and made `const`.

---

**`BudgetManager` — added explicit includes, `hasPendingBills()`**

`Expense`, `Bill`, and `Date` were used in method signatures but only available through transitive includes. Explicit `#include` directives were added for all three. `hasPendingBills() const` was added as a guard for `getNextBill()` — without it, calling `getNextBill()` on an empty heap would crash with no way for the caller to check first.

---

## Traceability Matrix

This section traces each user-facing requirement (the three core actions) through every layer of the design — from the action itself, to the components involved, to the specific header files, to the methods that fulfill it. Every method listed here maps directly to a declared signature in `include/`.

---

**Action 1: Log an expense and check if it exceeds a category budget**

- Components: `BudgetManager`, `BST`, `HashMap`, `CategoryInfo`, `Expense`
- Headers: `BudgetManager.h`, `BST.h`, `HashMap.h`, `CategoryInfo.h`, `Expense.h`
- Methods: `BudgetManager::addExpense()`, `BudgetManager::checkBudget()`, `BudgetManager::setBudgetLimit()`, `BST::insert()`, `HashMap::get()`, `HashMap::insert()`, `CategoryInfo::addExpense()`, `CategoryInfo::isOverBudget()`, `CategoryInfo::getRemainingBudget()`

---

**Action 2: View the next upcoming bill and mark it as paid**

- Components: `BudgetManager`, `MinHeap`, `Bill`
- Headers: `BudgetManager.h`, `MinHeap.h`, `Bill.h`
- Methods: `BudgetManager::hasPendingBills()`, `BudgetManager::getNextBill()`, `BudgetManager::markBillPaid()`, `MinHeap::peek()`, `MinHeap::markPaidByName()`, `Bill::markAsPaid()`, `Bill::display()`

---

**Action 3: Query spending history by date range**

- Components: `BudgetManager`, `BST`, `Expense`, `Date`
- Headers: `BudgetManager.h`, `BST.h`, `Expense.h`, `Date.h`
- Methods: `BudgetManager::getExpensesByRange()`, `BST::rangeQuery()`, `BST::rangeHelper()`, `Date::isBefore()`, `Date::isAfter()`, `Date::isBetween()`, `Expense::display()`

---

### Supporting Concerns

**Store and retrieve category budget limits**
- Components: `HashMap`, `CategoryInfo`
- Headers: `HashMap.h`, `CategoryInfo.h`
- Methods: `HashMap::insert()`, `HashMap::get()`, `HashMap::contains()`

**Prevent duplicate-date data loss in expense log**
- Component: `BST`
- Header: `BST.h`
- Method: `BST::insertHelper()` — equal dates go right so all expenses on the same day are preserved

**Prevent dangling pointer on bill lookup**
- Component: `MinHeap`
- Header: `MinHeap.h`
- Method: `MinHeap::markPaidByName()` — modifies bill in place, no re-heapify needed

**Prevent empty-heap crash**
- Components: `BudgetManager`, `MinHeap`
- Headers: `BudgetManager.h`, `MinHeap.h`
- Methods: `BudgetManager::hasPendingBills()`, `MinHeap::isEmpty()`

**Safe memory management for all raw pointers**
- Components: `HashMap`, `MinHeap`, `BST`
- Headers: `HashMap.h`, `MinHeap.h`, `BST.h`
- Methods: Destructors and Rule of Three — copy constructor and copy assignment operator deleted on all three

**Date comparison and formatting**
- Component: `Date`
- Header: `Date.h`
- Methods: `Date::operator==()`, `Date::isBefore()`, `Date::isAfter()`, `Date::isBetween()`, `Date::toString()`

---

### 1b. Design Changes (Implementation Phase)

A second round of design changes surfaced once implementation actually began. These were not header-review issues — they only became visible after the data structures were wired together and the UI demanded data the original API could not provide.

The most consequential change was that `BudgetManager.h` had no `getAll()` (or any iteration primitive) on `HashMap`, but `generateReport()` and the UI both need to walk every tracked category. Rather than retrofit `HashMap` with iteration (which would have widened its public surface), `BudgetManager.cpp` keeps a file-scope `static std::vector<std::string> s_categoryNames` that is appended on every new category insert. Reports and snapshots iterate this vector and look up each name via `HashMap::get()`. This is documented inline as a single-instance assumption — acceptable for portfolio scope, but worth flagging.

`getBudgetSnapshot()` was added to `BudgetManager` in Phase 5 (it does not appear in the original design). The ImGui panels need a value-copy view of every tracked category for live dashboard rendering, and the existing API only exposed budget data through `generateReport()`, which writes to `stdout`. `getBudgetSnapshot()` returns `std::vector<CategoryInfo>` so the renderer can iterate without holding `HashMap` internal pointers across frames (those pointers can be invalidated by a `resize()`).

The `Makefile` was rebuilt from scratch in Phase 1. The original used `g++` with no GUI flags, which would never have linked against ImGui or GLFW. The replacement uses `clang++ -std=c++17`, pulls flags from `pkg-config --libs glfw3`, links the macOS frameworks (`-framework OpenGL`, `-framework Cocoa`, `-framework IOKit`, `-framework CoreVideo`), and explicitly excludes `tests.cpp` from `APP_SRC` via `filter-out` so the test runner's `main()` does not collide with the app's `main()` at link time.

---

## 2. Implementation Reflection

### 2a. Data Structure Tradeoffs

**HashMap over BST for budgets.** Budget lookup happens on every single `addExpense()` call — this is the hottest path in the system. HashMap gives O(1) amortized lookup; a BST keyed by category name would have given O(log n) per lookup, which compounds quickly when bulk-importing expenses. Category names are short ASCII strings, and the djb2 hash distributes them evenly across the open-addressed table, so collisions stay rare in practice. The tradeoff is real, though: HashMap has no natural ordering and no iteration primitive, which is exactly why `BudgetManager.cpp` had to keep the file-scope `s_categoryNames` registry to support `generateReport()` and `getBudgetSnapshot()`. A balanced BST would have given iteration for free at the cost of every lookup being slower — the wrong tradeoff for this access pattern.

**MinHeap over sorted vector for bills.** The system always needs the *next-due* bill — the minimum due date — never a fully sorted list. MinHeap gives O(1) `peek()` and O(log n) `insert()`; a sorted vector would force every `insert()` to do an O(n) shift to maintain ordering, which is strictly worse on the dominant operation. The tradeoff is that `markPaidByName()` requires an O(n) linear scan over the heap array since the heap is indexed by priority, not by name. For portfolio scope (typically <50 bills) this is a non-issue, but at scale the right answer would be a heap paired with a side hash map from name to heap index.

**BST over vector/deque for expenses.** Expense history is queried by date range, not by insertion order, so a vector would force every `getExpensesByRange()` call to do an O(n) full scan. The BST's `rangeQuery()` prunes entire subtrees in O(log n + k), where k is the number of results returned — the cost scales with the answer size, not the dataset size. The tradeoff is that this BST is not self-balancing: if expenses arrive in chronological order (the common case in a real app where the user logs today's expenses today), the tree degenerates into a right-leaning chain and inserts approach O(n). A self-balancing tree (red-black, AVL) would fix this; the portfolio scope accepts the degeneracy as a documented limitation.

### 2b. Algorithm Notes — BST Range Query

`rangeHelper()` is the core of the BST's range-query advantage over a flat scan, and it's worth understanding why the pruning works. At each node, the helper compares the node's date against the `[start, end]` window before deciding which subtrees to descend into. If `node->date.isBefore(start)`, then by the BST invariant the entire left subtree is also before `start` — every node there is strictly smaller — so we skip it and recurse only right. Symmetrically, if `node->date.isAfter(end)`, the entire right subtree is also after `end` and we recurse only left. Otherwise the node sits inside the window: we recurse left, append the node to the result, then recurse right. This pruning is what gives O(log n + k) instead of O(n) — we only ever visit nodes that are either ancestors of an in-range result or in-range themselves.

---

## 3. AI Strategy

Claude Code (Opus 4.7) was used as the primary implementation collaborator across all seven phases of the project. It generated the initial header skeletons from the UML diagram, implemented every data structure against precise behavioral contracts, built the five ImGui panels, wrote the test suite (`tests.cpp`), and produced the Makefile that targets Apple Silicon clang++ with GLFW/OpenGL linkage. The lead architect role on my side was specifying contracts, reviewing diffs, and resolving spec divergences — the AI did the keystrokes.

The two most useful Claude Code features for this project were **parallel sub-agent execution** and **git worktree isolation**. Wave 2 dispatched three sub-agents simultaneously — one each for `HashMap.cpp`, `MinHeap.cpp`, and `BST.cpp` — and they ran to completion in isolated worktrees without ever touching the same file. Without worktrees, three concurrent agents writing into the same `src/` directory would have raced on the Makefile and headers; with them, each agent had a private checkout and the merges were trivial. This cut wall-clock time on Wave 2 from sequential (likely 30+ minutes) to roughly a third of that.

One AI suggestion was rejected and a substitute was kept. The original `instructions.md` spec called for `HashMap` to use **separate chaining with exactly 101 buckets**. The submitted design instead uses **open-addressing with linear probing and tombstone deletion**, sized at 16 buckets initial capacity and doubling on load factor > 0.7. The open-addressing design was retained because it had already been formally submitted and justified at the design-review stage, and switching backing strategies mid-implementation would have invalidated the traceability between the design document and the implementation. This is a clean example of where a stylized spec rule had to defer to a real-world commitment.

The lead-architect judgment calls — adding `getBudgetSnapshot()` when the original header made the UI impossible, accepting the `s_categoryNames` workaround, ordering Wave 2 to minimize inter-agent dependencies, choosing to vendor ImGui rather than treat it as an external dep — are the parts that the AI could *propose* but could not *decide*. Those decisions are documented in the agent memory directory under `.claude/agent-memory/cpp-finance-architect/` and will inform any future maintenance pass on this codebase.
