// tests.cpp -- Phase 6 standalone test suite for the Finance Manager
// custom data structures and BudgetManager controller.
//
// Compiled WITHOUT ImGui/GLFW. See `make test` in the Makefile.
// Each test function asserts behavior and prints "PASS: <name>" on success.
// A failed assert aborts with the assertion text + line number, which is
// the contract expected by the project's Phase 6 test plan.

#include <cassert>
#include <iostream>
#include <string>
#include <vector>

#include "Date.h"
#include "Expense.h"
#include "Bill.h"
#include "CategoryInfo.h"
#include "HashMap.h"
#include "MinHeap.h"
#include "BST.h"
#include "BudgetManager.h"

// ---------------------------------------------------------------------------
// 1. Date
// ---------------------------------------------------------------------------
static void testDate() {
    // isBefore: strict ordering
    assert(Date(1, 4, 2025).isBefore(Date(2, 4, 2025)) == true);
    assert(Date(2, 4, 2025).isBefore(Date(1, 4, 2025)) == false);
    // Equal dates are not "before" each other (strict).
    assert(Date(1, 4, 2025).isBefore(Date(1, 4, 2025)) == false);

    // isAfter: month rollover (Apr 1 vs Mar 31)
    assert(Date(1, 4, 2025).isAfter(Date(31, 3, 2025)) == true);

    // isBetween: inclusive on both endpoints, exclusive of out-of-range dates
    assert(Date(15, 6, 2025).isBetween(Date(1, 6, 2025), Date(30, 6, 2025)) == true);
    assert(Date(31, 5, 2025).isBetween(Date(1, 6, 2025), Date(30, 6, 2025)) == false);

    // toString: zero-padded YYYY-MM-DD
    assert(Date(1, 1, 2025).toString() == "2025-01-01");
    assert(Date(15, 12, 2024).toString() == "2024-12-15");

    // operator==
    assert((Date(5, 3, 2025) == Date(5, 3, 2025)) == true);
    assert((Date(5, 3, 2025) == Date(6, 3, 2025)) == false);

    std::cout << "PASS: testDate" << std::endl;
}

// ---------------------------------------------------------------------------
// 2. CategoryInfo
// ---------------------------------------------------------------------------
static void testCategoryInfo() {
    CategoryInfo c;
    // Default constructor zero-initializes the budget bookkeeping.
    assert(c.budgetLimit == 0.0);
    assert(c.totalSpent == 0.0);

    c.addExpense(100.0);
    assert(c.totalSpent == 100.0);
    c.addExpense(50.0);
    assert(c.totalSpent == 150.0);

    c.budgetLimit = 200.0;
    assert(c.getRemainingBudget() == 50.0);
    assert(c.isOverBudget() == false);

    c.addExpense(100.0); // totalSpent now 250 vs 200 limit
    assert(c.totalSpent == 250.0);
    assert(c.isOverBudget() == true);

    std::cout << "PASS: testCategoryInfo" << std::endl;
}

// ---------------------------------------------------------------------------
// 3. HashMap
// ---------------------------------------------------------------------------
static void testHashMap() {
    HashMap map;

    CategoryInfo food;
    food.name = "Food";
    food.budgetLimit = 500.0;
    map.insert("Food", food);

    assert(map.contains("Food") == true);
    {
        CategoryInfo* p = map.get("Food");
        assert(p != nullptr);
        assert(p->budgetLimit == 500.0);
    }

    // Negative lookups: contains() false, get() returns nullptr
    assert(map.contains("NonExistent") == false);
    assert(map.get("NonExistent") == nullptr);

    // Insert with the same key updates the existing entry in place
    // (size stays 1 -- not directly observable, but resize-test below
    // also covers no-spurious-grow behavior).
    CategoryInfo food2;
    food2.name = "Food";
    food2.budgetLimit = 600.0;
    map.insert("Food", food2);
    {
        CategoryInfo* p = map.get("Food");
        assert(p != nullptr);
        assert(p->budgetLimit == 600.0);
    }

    // Tombstone-aware deletion
    map.remove("Food");
    assert(map.contains("Food") == false);
    assert(map.get("Food") == nullptr);

    // Resize stress: insert 20 entries, force at least one resize
    // (initial capacity 16, load factor 0.7 triggers resize at ~12).
    // After resize, every key must still be findable.
    HashMap big;
    for (int i = 0; i < 20; ++i) {
        std::string key = "Cat" + std::to_string(i);
        CategoryInfo info;
        info.name = key;
        info.budgetLimit = i * 10.0;
        big.insert(key, info);
    }
    for (int i = 0; i < 20; ++i) {
        std::string key = "Cat" + std::to_string(i);
        assert(big.contains(key) == true);
        CategoryInfo* p = big.get(key);
        assert(p != nullptr);
        assert(p->budgetLimit == i * 10.0);
    }

    std::cout << "PASS: testHashMap" << std::endl;
}

// ---------------------------------------------------------------------------
// 4. MinHeap
// ---------------------------------------------------------------------------
static void testMinHeap() {
    MinHeap heap;

    Bill b1; b1.name = "Rent";     b1.dueDate = Date(1, 5, 2025);  b1.amountDue = 1200.0;
    Bill b2; b2.name = "Internet"; b2.dueDate = Date(15, 4, 2025); b2.amountDue = 60.0;
    Bill b3; b3.name = "Phone";    b3.dueDate = Date(3, 4, 2025);  b3.amountDue = 45.0;

    assert(heap.isEmpty() == true);

    // Insert in arbitrary order; heap orders by dueDate.
    heap.insert(b1);
    heap.insert(b2);
    heap.insert(b3);

    // Apr 3 is the earliest of the three.
    assert(heap.peek().name == "Phone");

    assert(heap.extractMin().name == "Phone");
    assert(heap.peek().name == "Internet");
    assert(heap.extractMin().name == "Internet");
    assert(heap.extractMin().name == "Rent");
    assert(heap.isEmpty() == true);

    // markPaidByName: returns true on hit, false on miss.
    // Re-insert Rent (May 1) and Internet (Apr 15). Internet is at the top.
    heap.insert(b1);
    heap.insert(b2);

    assert(heap.markPaidByName("Rent", Date(30, 4, 2025)) == true);
    assert(heap.markPaidByName("Ghost", Date(1, 1, 2025)) == false);

    // Verify Rent's isPaid flag was actually set. The heap has no random
    // access getter, so we extract the top (Internet), then peek the next
    // (Rent) and confirm.
    Bill top = heap.extractMin();
    assert(top.name == "Internet");
    Bill nextRent = heap.peek();
    assert(nextRent.name == "Rent");
    assert(nextRent.isPaid == true);
    assert(nextRent.paidOn == Date(30, 4, 2025));

    std::cout << "PASS: testMinHeap" << std::endl;
}

// ---------------------------------------------------------------------------
// 5. BST
// ---------------------------------------------------------------------------
static void testBST() {
    BST tree;

    Expense e1; e1.date = Date(1, 4, 2025); e1.category = "Food";      e1.amount = 100.0;
    Expense e2; e2.date = Date(5, 4, 2025); e2.category = "Transport"; e2.amount = 50.0;
    Expense e3; e3.date = Date(3, 4, 2025); e3.category = "Food";      e3.amount = 75.0;
    // Duplicate date with e1 -- the BST inserts equal dates to the right
    // so no expense is silently dropped.
    Expense e4; e4.date = Date(1, 4, 2025); e4.category = "Food";      e4.amount = 30.0;

    tree.insert(e1);
    tree.insert(e2);
    tree.insert(e3);
    tree.insert(e4);

    std::vector<Expense> in = tree.inOrder();
    assert(in.size() == 4);

    // Endpoints: earliest first, latest last.
    assert(in.front().date == Date(1, 4, 2025));
    assert(in.back().date == Date(5, 4, 2025));

    // Sorted non-decreasing: use !isAfter to express <=.
    for (size_t i = 0; i + 1 < in.size(); ++i) {
        assert(!in[i].date.isAfter(in[i + 1].date));
    }

    // rangeQuery: inclusive endpoints.
    {
        auto r = tree.rangeQuery(Date(1, 4, 2025), Date(3, 4, 2025));
        assert(r.size() == 3); // Apr 1 (x2) + Apr 3
    }
    {
        auto r = tree.rangeQuery(Date(4, 4, 2025), Date(5, 4, 2025));
        assert(r.size() == 1); // just Apr 5
    }
    {
        auto r = tree.rangeQuery(Date(10, 4, 2025), Date(20, 4, 2025));
        assert(r.size() == 0);
    }

    // search: returns every expense on a given date (handles duplicates).
    {
        auto s = tree.search(Date(1, 4, 2025));
        assert(s.size() == 2);
    }
    {
        auto s = tree.search(Date(5, 4, 2025));
        assert(s.size() == 1);
    }

    std::cout << "PASS: testBST" << std::endl;
}

// ---------------------------------------------------------------------------
// 6. BudgetManager
// ---------------------------------------------------------------------------
// NOTE: BudgetManager.cpp keeps a file-scope static category-name registry
// shared across instances. We use exactly ONE manager here to avoid leaking
// category names between test runs.
static void testBudgetManager() {
    BudgetManager manager;

    manager.setBudgetLimit("Food", 500.0);

    Expense e1;
    e1.category = "Food";
    e1.amount = 400.0;
    e1.date = Date(1, 4, 2025);
    e1.description = "Groceries";
    manager.addExpense(e1);

    auto apr = manager.getExpensesByRange(Date(1, 4, 2025), Date(30, 4, 2025));
    assert(apr.size() == 1);

    Expense e2;
    e2.category = "Food";
    e2.amount = 50.0;
    e2.date = Date(10, 4, 2025);
    e2.description = "Snack";
    manager.addExpense(e2);

    apr = manager.getExpensesByRange(Date(1, 4, 2025), Date(30, 4, 2025));
    assert(apr.size() == 2);

    // Bills queue starts empty.
    assert(manager.hasPendingBills() == false);

    Bill rent;
    rent.name = "Rent";
    rent.amountDue = 1200.0;
    rent.dueDate = Date(1, 5, 2025);

    Bill internet;
    internet.name = "Internet";
    internet.amountDue = 60.0;
    internet.dueDate = Date(15, 4, 2025);

    manager.addBill(rent);
    manager.addBill(internet);

    assert(manager.hasPendingBills() == true);
    // Apr 15 (Internet) < May 1 (Rent) -- Internet is the next due bill.
    assert(manager.getNextBill().name == "Internet");

    // markBillPaid flips the flag in place but does not re-heapify, so the
    // heap top is unchanged. We only verify the contract that getNextBill()
    // is still callable afterwards and still returns "Internet".
    manager.markBillPaid("Internet", Date(14, 4, 2025));
    assert(manager.getNextBill().name == "Internet");

    std::cout << "PASS: testBudgetManager" << std::endl;
}

// ---------------------------------------------------------------------------
// 7. Date -- edge cases
// ---------------------------------------------------------------------------
static void testDateEdgeCases() {
    // Cross-year boundary
    assert(Date(31, 12, 2025).isBefore(Date(1, 1, 2026)) == true);
    assert(Date(1, 1, 2026).isAfter(Date(31, 12, 2025)) == true);

    // isBetween: both endpoints are inclusive
    assert(Date(1,  6, 2025).isBetween(Date(1, 6, 2025), Date(30, 6, 2025)) == true);
    assert(Date(30, 6, 2025).isBetween(Date(1, 6, 2025), Date(30, 6, 2025)) == true);
    assert(Date(31, 5, 2025).isBetween(Date(1, 6, 2025), Date(30, 6, 2025)) == false);
    assert(Date(1,  7, 2025).isBetween(Date(1, 6, 2025), Date(30, 6, 2025)) == false);

    // toString: single-digit day and month are zero-padded
    assert(Date(5, 7, 2025).toString() == "2025-07-05");
    assert(Date(9, 9, 2025).toString() == "2025-09-09");

    std::cout << "PASS: testDateEdgeCases" << std::endl;
}

// ---------------------------------------------------------------------------
// 8. CategoryInfo -- edge cases
// ---------------------------------------------------------------------------
static void testCategoryInfoEdgeCases() {
    // getRemainingBudget is negative when over-spent
    CategoryInfo c;
    c.budgetLimit = 100.0;
    c.addExpense(150.0);
    assert(c.getRemainingBudget() == -50.0);
    assert(c.isOverBudget() == true);

    // Zero budget limit: any expense pushes it over
    CategoryInfo z;
    z.budgetLimit = 0.0;
    z.addExpense(0.01);
    assert(z.isOverBudget() == true);

    std::cout << "PASS: testCategoryInfoEdgeCases" << std::endl;
}

// ---------------------------------------------------------------------------
// 9. HashMap -- tombstone reuse and partial removal
// ---------------------------------------------------------------------------
static void testHashMapEdgeCases() {
    // Insert -> remove -> re-insert same key must work (tombstone reuse)
    HashMap map;
    CategoryInfo a; a.name = "A"; a.budgetLimit = 10.0;
    map.insert("A", a);
    map.remove("A");
    assert(map.contains("A") == false);

    CategoryInfo a2; a2.name = "A"; a2.budgetLimit = 20.0;
    map.insert("A", a2);
    assert(map.contains("A") == true);
    assert(map.get("A")->budgetLimit == 20.0);

    // Removing one key from a multi-key map leaves neighbors intact
    HashMap m;
    for (int i = 0; i < 5; ++i) {
        CategoryInfo ci; ci.name = "K" + std::to_string(i); ci.budgetLimit = i * 5.0;
        m.insert(ci.name, ci);
    }
    m.remove("K2");
    assert(m.contains("K2") == false);
    assert(m.contains("K0") == true);
    assert(m.contains("K4") == true);
    assert(m.get("K3")->budgetLimit == 15.0);

    std::cout << "PASS: testHashMapEdgeCases" << std::endl;
}

// ---------------------------------------------------------------------------
// 10. MinHeap -- removeByName, getAllBills, empty-heap checks
// ---------------------------------------------------------------------------
static void testMinHeapEdgeCases() {
    // getAllBills on an empty heap returns an empty vector
    MinHeap empty;
    assert(empty.getAllBills().empty() == true);

    // removeByName: hit returns true and removes; miss returns false
    MinHeap h;
    Bill b1; b1.name = "Alpha"; b1.dueDate = Date(10, 5, 2025); b1.amountDue = 100.0;
    Bill b2; b2.name = "Beta";  b2.dueDate = Date(20, 5, 2025); b2.amountDue = 200.0;
    Bill b3; b3.name = "Gamma"; b3.dueDate = Date(1,  6, 2025); b3.amountDue = 50.0;
    h.insert(b1); h.insert(b2); h.insert(b3);

    assert(h.removeByName("Beta")  == true);
    assert(h.removeByName("Ghost") == false);

    // getAllBills reflects the removal
    assert(h.getAllBills().size() == 2);

    // Extraction order after removeByName still respects heap order
    assert(h.extractMin().name == "Alpha");
    assert(h.extractMin().name == "Gamma");
    assert(h.isEmpty() == true);

    std::cout << "PASS: testMinHeapEdgeCases" << std::endl;
}

// ---------------------------------------------------------------------------
// 11. BST -- empty tree, single node, left-heavy spine
// ---------------------------------------------------------------------------
static void testBSTEdgeCases() {
    // Empty tree: all queries return empty
    BST emptyTree;
    assert(emptyTree.inOrder().empty() == true);
    assert(emptyTree.search(Date(1, 1, 2025)).empty() == true);
    assert(emptyTree.rangeQuery(Date(1, 1, 2025), Date(31, 12, 2025)).empty() == true);

    // Single-node tree
    BST single;
    Expense e; e.date = Date(15, 6, 2025); e.category = "Food"; e.amount = 50.0;
    single.insert(e);
    assert(single.inOrder().size() == 1);
    assert(single.search(Date(15, 6, 2025)).size() == 1);
    assert(single.search(Date(16, 6, 2025)).empty() == true);
    assert(single.rangeQuery(Date(1,  6, 2025), Date(30, 6, 2025)).size() == 1);
    assert(single.rangeQuery(Date(1,  7, 2025), Date(31, 7, 2025)).empty() == true);

    // Descending-date insertions produce a left-heavy spine; inOrder still sorts correctly
    BST spine;
    for (int d = 5; d >= 1; --d) {
        Expense ex; ex.date = Date(d, 4, 2025); ex.amount = d * 10.0; ex.category = "X";
        spine.insert(ex);
    }
    auto io = spine.inOrder();
    assert(io.size() == 5);
    for (size_t i = 0; i + 1 < io.size(); ++i)
        assert(!io[i].date.isAfter(io[i + 1].date));

    std::cout << "PASS: testBSTEdgeCases" << std::endl;
}

// ---------------------------------------------------------------------------
// 12. BudgetManager -- monthly limits, getSpentInMonth, payNextBill,
//     removeBill, getAllBills, getBudgetSnapshot
// ---------------------------------------------------------------------------
// NOTE: s_categoryNames and s_monthlyLimits are file-scope statics shared
// across all BudgetManager instances. Use category names not used elsewhere
// ("Dining", "Utilities") to avoid cross-test contamination.
static void testBudgetManagerExtended() {
    BudgetManager m;

    // Monthly limit overload and getLimitForMonth
    m.setBudgetLimit("Dining", 300.0, 4, 2025);
    assert(m.getLimitForMonth("Dining", 4, 2025) == 300.0);
    // A month with no override and no global default returns 0.0 (not set)
    assert(m.getLimitForMonth("Dining", 5, 2025) == 0.0);

    // getSpentInMonth only counts expenses in the right month AND category
    Expense e1; e1.category = "Dining"; e1.amount = 120.0; e1.date = Date(5,  4, 2025);
    Expense e2; e2.category = "Dining"; e2.amount = 80.0;  e2.date = Date(20, 4, 2025);
    Expense e3; e3.category = "Dining"; e3.amount = 50.0;  e3.date = Date(1,  5, 2025);
    m.addExpense(e1); m.addExpense(e2); m.addExpense(e3);
    assert(m.getSpentInMonth("Dining", 4, 2025) == 200.0);
    assert(m.getSpentInMonth("Dining", 5, 2025) == 50.0);

    // getBudgetSnapshotForMonth reflects per-month spending and limits
    auto snap = m.getBudgetSnapshotForMonth(4, 2025);
    bool found = false;
    for (const auto& ci : snap) {
        if (ci.name == "Dining") {
            assert(ci.totalSpent  == 200.0);
            assert(ci.budgetLimit == 300.0);
            found = true;
        }
    }
    assert(found == true);

    // getBudgetSnapshot returns global totals (all three expenses = 250)
    auto global = m.getBudgetSnapshot();
    found = false;
    for (const auto& ci : global) {
        if (ci.name == "Dining") {
            assert(ci.totalSpent == 250.0);
            found = true;
        }
    }
    assert(found == true);

    // payNextBill: marks paid AND removes from heap so the next bill advances
    Bill b1; b1.name = "Utilities"; b1.dueDate = Date(5,  4, 2025); b1.amountDue = 80.0;
    Bill b2; b2.name = "Streaming"; b2.dueDate = Date(15, 4, 2025); b2.amountDue = 15.0;
    m.addBill(b1); m.addBill(b2);
    assert(m.getNextBill().name == "Utilities");
    m.payNextBill(Date(4, 4, 2025));
    assert(m.getNextBill().name == "Streaming");

    // removeBill: removes a specific bill by name; heap becomes empty
    m.removeBill("Streaming", Date(14, 4, 2025));
    assert(m.hasPendingBills() == false);

    // getAllBills reflects current heap contents
    Bill b3; b3.name = "Gym"; b3.dueDate = Date(1, 5, 2025); b3.amountDue = 40.0;
    m.addBill(b3);
    auto allBills = m.getAllBills();
    assert(allBills.size() == 1);
    assert(allBills[0].name == "Gym");

    std::cout << "PASS: testBudgetManagerExtended" << std::endl;
}

// ---------------------------------------------------------------------------
int main() {
    testDate();
    testCategoryInfo();
    testHashMap();
    testMinHeap();
    testBST();
    testBudgetManager();
    testDateEdgeCases();
    testCategoryInfoEdgeCases();
    testHashMapEdgeCases();
    testMinHeapEdgeCases();
    testBSTEdgeCases();
    testBudgetManagerExtended();
    std::cout << "\nAll tests passed." << std::endl;
    return 0;
}
