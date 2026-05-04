// BudgetManager.cpp — integrates HashMap (category lookup),
// MinHeap (next-due bill), and BST (date-sorted expenses).
//
// HashMap exposes no iterator/getAll, so this translation unit keeps a
// file-scope vector of every category name we have ever inserted. We
// dedupe on insert so the report walks each category once. The vector
// is module-static rather than a class member because we can't change
// the BudgetManager.h contract.

#include "BudgetManager.h"

#include <cstdio>
#include <iostream>
#include <iomanip>

// File-scope registry of category names tracked by BudgetManager.
// NOTE: this is a single-instance assumption -- if the app ever holds
// two BudgetManager objects, their category lists will collide. The
// portfolio scope is a single manager, so this is acceptable.
static std::vector<std::string> s_categoryNames;

// Pushes name into s_categoryNames only if it isn't already present.
// O(n) over n = number of categories, which is small (<20 in practice).
static void rememberCategory(const std::string& name) {
    for (const auto& existing : s_categoryNames) {
        if (existing == name) {
            return;
        }
    }
    s_categoryNames.push_back(name);
}

void BudgetManager::setBudgetLimit(const std::string& category, double limit) {
    if (!categoryMap.contains(category)) {
        CategoryInfo info;
        info.name = category;
        info.budgetLimit = limit;
        info.totalSpent = 0.0;
        categoryMap.insert(category, info);
        rememberCategory(category);
        return;
    }

    // Already exists: update the budget ceiling but preserve totalSpent
    // so a mid-period budget adjustment doesn't reset accumulated history.
    CategoryInfo* info = categoryMap.get(category);
    if (info != nullptr) {
        info->budgetLimit = limit;
    }
}

void BudgetManager::addExpense(const Expense& e) {
    // Auto-register categories that receive an expense before any
    // setBudgetLimit() call. budgetLimit=0.0 marks "no limit set" --
    // checkBudget() detects this case and skips threshold math.
    if (!categoryMap.contains(e.category)) {
        CategoryInfo info;
        info.name = e.category;
        info.budgetLimit = 0.0;
        info.totalSpent = 0.0;
        categoryMap.insert(e.category, info);
        rememberCategory(e.category);
    }

    // Re-fetch pointer AFTER any insert to avoid acting on an address
    // that resize() may have invalidated. addExpense on CategoryInfo
    // does not touch the HashMap, so the pointer is stable across that call.
    CategoryInfo* info = categoryMap.get(e.category);
    if (info != nullptr) {
        info->addExpense(e.amount);
    }

    expenseTree.insert(e);
}

void BudgetManager::addBill(const Bill& b) {
    billHeap.insert(b);
}

void BudgetManager::checkBudget(const std::string& category) {
    if (!categoryMap.contains(category)) {
        std::cout << "Category not found: " << category << std::endl;
        return;
    }

    CategoryInfo* info = categoryMap.get(category);
    if (info == nullptr) {
        std::cout << "Category not found: " << category << std::endl;
        return;
    }

    if (info->budgetLimit == 0.0) {
        std::cout << "No budget set for: " << category << std::endl;
        return;
    }

    double pct = (info->totalSpent / info->budgetLimit) * 100.0;

    // Format dollar amounts with 2 decimals; percent with 1 decimal.
    std::cout << std::fixed << std::setprecision(2);

    if (pct > 100.0) {
        std::cout << "EXCEEDED: " << category
                  << " -- spent $" << info->totalSpent
                  << " of $" << info->budgetLimit
                  << " (" << std::setprecision(1) << pct << "%)"
                  << std::setprecision(2) << std::endl;
    } else if (pct >= 80.0) {
        std::cout << "WARNING: " << category
                  << " -- spent $" << info->totalSpent
                  << " of $" << info->budgetLimit
                  << " (" << std::setprecision(1) << pct << "%)"
                  << std::setprecision(2) << std::endl;
    } else {
        std::cout << "OK: " << category
                  << " -- spent $" << info->totalSpent
                  << " of $" << info->budgetLimit
                  << " (" << std::setprecision(1) << pct << "%)"
                  << std::setprecision(2) << std::endl;
    }
}

bool BudgetManager::hasPendingBills() const {
    return !billHeap.isEmpty();
}

Bill BudgetManager::getNextBill() {
    // Caller is expected to gate this on hasPendingBills(); MinHeap::peek()
    // on an empty heap is its own contract problem, not ours to retry here.
    return billHeap.peek();
}

void BudgetManager::markBillPaid(const std::string& name, const Date& paymentDate) {
    billHeap.markPaidByName(name, paymentDate);
}

std::vector<Expense> BudgetManager::getExpensesByRange(const Date& start, const Date& end) {
    return expenseTree.rangeQuery(start, end);
}

void BudgetManager::generateReport() {
    std::cout << "===== Budget Report =====" << std::endl;

    if (s_categoryNames.empty()) {
        std::cout << "No categories tracked." << std::endl;
        std::cout << "=========================" << std::endl;
        return;
    }

    std::cout << std::fixed << std::setprecision(2);
    for (const auto& name : s_categoryNames) {
        CategoryInfo* info = categoryMap.get(name);
        if (info == nullptr) {
            // Defensive: registry knows the name but map lost it.
            std::cout << "  " << name << " -- (missing)" << std::endl;
            continue;
        }

        double remaining = info->budgetLimit - info->totalSpent;
        const char* status;
        if (info->budgetLimit == 0.0) {
            status = "no-limit";
        } else if (info->totalSpent > info->budgetLimit) {
            status = "OVER";
        } else {
            status = "under";
        }

        std::cout << "  " << name
                  << " | limit: $" << info->budgetLimit
                  << " | spent: $" << info->totalSpent
                  << " | remaining: $" << remaining
                  << " | " << status
                  << std::endl;
    }
    std::cout << "=========================" << std::endl;
}

// UI helper: snapshot every tracked category as a value-copy vector so the
// renderer can iterate without touching internal HashMap pointers. Non-const
// because HashMap::get() is non-const.
std::vector<CategoryInfo> BudgetManager::getBudgetSnapshot() {
    std::vector<CategoryInfo> result;
    for (const auto& name : s_categoryNames) {
        CategoryInfo* p = categoryMap.get(name);
        if (p != nullptr) {
            result.push_back(*p);
        }
    }
    return result;
}

void BudgetManager::payNextBill(const Date& paymentDate) {
    if (!billHeap.isEmpty()) {
        billHeap.markPaidByName(billHeap.peek().name, paymentDate);
        billHeap.extractMin();
    }
}

std::vector<Bill> BudgetManager::getAllBills() {
    return billHeap.getAllBills();
}

void BudgetManager::removeBill(const std::string& name, const Date& paymentDate) {
    billHeap.markPaidByName(name, paymentDate);
    billHeap.removeByName(name);
}

double BudgetManager::getSpentInMonth(const std::string& category, int month, int year) {
    Date start(1, month, year);
    Date end(31, month, year);  // day=31 is a safe ceiling; month comparison precedes day
    auto expenses = expenseTree.rangeQuery(start, end);
    double total = 0.0;
    for (const auto& e : expenses) {
        if (e.category == category) total += e.amount;
    }
    return total;
}

std::vector<CategoryInfo> BudgetManager::getBudgetSnapshotForMonth(int month, int year) {
    std::vector<CategoryInfo> result;
    for (const auto& name : s_categoryNames) {
        CategoryInfo* p = categoryMap.get(name);
        if (p != nullptr) {
            CategoryInfo copy = *p;
            copy.totalSpent = getSpentInMonth(name, month, year);
            result.push_back(copy);
        }
    }
    return result;
}
