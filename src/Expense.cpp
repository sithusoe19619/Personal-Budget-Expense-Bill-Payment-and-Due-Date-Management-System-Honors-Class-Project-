#include "Expense.h"
#include <iostream>

void Expense::display() const {
    std::cout << "[" << date.toString() << "] "
              << category << " | "
              << amount << " | "
              << description << std::endl;
}
