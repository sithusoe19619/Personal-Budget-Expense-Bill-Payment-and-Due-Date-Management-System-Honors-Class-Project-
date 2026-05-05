#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <vector>
#include <sqlite3.h>
#include "Expense.h"
#include "Bill.h"

class Database {
public:
    struct BudgetLimitRow {
        std::string category;
        double      limit;
        int         month;
        int         year;
    };

    explicit Database(const std::string& path);
    ~Database();
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    // Auth — returns user id (>= 1) on success, -1 on failure
    int registerUser(const std::string& username, const std::string& password);
    int loginUser   (const std::string& username, const std::string& password);

    // Load all data for a user (called once on login to hydrate BudgetManager)
    std::vector<Expense>        loadExpenses    (int userId);
    std::vector<Bill>           loadBills       (int userId);
    std::vector<BudgetLimitRow> loadBudgetLimits(int userId);

    // Write-through (called alongside every BudgetManager mutation)
    void insertExpense    (int userId, const Expense& e);
    void insertBill       (int userId, const Bill& b);
    void upsertBudgetLimit(int userId, const std::string& category,
                           double limit, int month, int year);
    void deleteBill       (int userId, const std::string& name);

private:
    sqlite3* db_;
    void               initSchema();
    static std::string hashPassword(const std::string& password);
};

#endif // DATABASE_H
