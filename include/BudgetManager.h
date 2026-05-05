#ifndef BUDGETMANAGER_H
#define BUDGETMANAGER_H

#include <string>
#include <vector>
#include "BST.h"
#include "MinHeap.h"
#include "HashMap.h"
#include "Expense.h"
#include "Bill.h"
#include "Date.h"
#include "CategoryInfo.h"

class BudgetManager {
private:
    BST expenseTree;
    MinHeap billHeap;
    HashMap categoryMap;

public:
    void addExpense(const Expense& e);
    void addBill(const Bill& b);
    void setBudgetLimit(const std::string& category, double limit);
    void setBudgetLimit(const std::string& category, double limit, int month, int year);
    void checkBudget(const std::string& category);
    bool hasPendingBills() const;  // call before getNextBill() to avoid empty-heap crash
    Bill getNextBill();
    void markBillPaid(const std::string& name, const Date& paymentDate);
    void payNextBill(const Date& paymentDate); // marks paid AND removes from heap
    std::vector<Expense> getExpensesByRange(const Date& start, const Date& end);
    void generateReport();
    std::vector<CategoryInfo> getBudgetSnapshot();
    std::vector<Bill> getAllBills();
    void removeBill(const std::string& name, const Date& paymentDate);
    double getSpentInMonth(const std::string& category, int month, int year);
    double getLimitForMonth(const std::string& category, int month, int year);
    std::vector<CategoryInfo> getBudgetSnapshotForMonth(int month, int year);
    static void clearStatics(); // call on logout to wipe file-scope state
};

#endif // BUDGETMANAGER_H
