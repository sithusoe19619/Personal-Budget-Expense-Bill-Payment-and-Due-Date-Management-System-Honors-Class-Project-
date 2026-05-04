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
int main() {
    testDate();
    testCategoryInfo();
    testHashMap();
    testMinHeap();
    testBST();
    testBudgetManager();
    std::cout << "\nAll tests passed." << std::endl;
    return 0;
}
