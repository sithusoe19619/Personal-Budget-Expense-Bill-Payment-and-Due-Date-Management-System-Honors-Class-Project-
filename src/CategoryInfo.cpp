#include "CategoryInfo.h"

void CategoryInfo::addExpense(double amount) {
    totalSpent += amount;
}

double CategoryInfo::getRemainingBudget() const {
    return budgetLimit - totalSpent;
}

bool CategoryInfo::isOverBudget() const {
    return totalSpent > budgetLimit;
}
