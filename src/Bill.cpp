#include "Bill.h"
#include <iostream>

void Bill::markAsPaid(const Date& paymentDate) {
    isPaid = true;
    paidOn = paymentDate;
}

void Bill::display() const {
    std::cout << "Bill: " << name
              << " | Due: " << dueDate.year << "-" << dueDate.month << "-" << dueDate.day
              << " | Amount: $" << amountDue
              << " | Paid: " << (isPaid ? "yes" : "no")
              << std::endl;
}
