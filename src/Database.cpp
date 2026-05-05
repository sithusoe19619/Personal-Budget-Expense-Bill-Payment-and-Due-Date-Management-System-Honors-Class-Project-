#include "Database.h"
#include <CommonCrypto/CommonDigest.h>
#include <cstdio>
#include <stdexcept>

Database::Database(const std::string& path) : db_(nullptr) {
    if (sqlite3_open(path.c_str(), &db_) != SQLITE_OK) {
        throw std::runtime_error(std::string("Cannot open database: ") + sqlite3_errmsg(db_));
    }
    // Enable WAL mode and foreign keys for safety.
    sqlite3_exec(db_, "PRAGMA journal_mode=WAL;", nullptr, nullptr, nullptr);
    sqlite3_exec(db_, "PRAGMA foreign_keys=ON;",  nullptr, nullptr, nullptr);
    initSchema();
}

Database::~Database() {
    if (db_) sqlite3_close(db_);
}

void Database::initSchema() {
    const char* sql =
        "CREATE TABLE IF NOT EXISTS users ("
        "  id            INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  username      TEXT    UNIQUE NOT NULL,"
        "  password_hash TEXT    NOT NULL"
        ");"

        "CREATE TABLE IF NOT EXISTS expenses ("
        "  id          INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  user_id     INTEGER NOT NULL,"
        "  category    TEXT    NOT NULL,"
        "  amount      REAL    NOT NULL,"
        "  day         INTEGER NOT NULL,"
        "  month       INTEGER NOT NULL,"
        "  year        INTEGER NOT NULL,"
        "  description TEXT    NOT NULL DEFAULT '',"
        "  FOREIGN KEY (user_id) REFERENCES users(id)"
        ");"

        "CREATE TABLE IF NOT EXISTS bills ("
        "  id         INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  user_id    INTEGER NOT NULL,"
        "  name       TEXT    NOT NULL,"
        "  amount_due REAL    NOT NULL,"
        "  due_day    INTEGER NOT NULL,"
        "  due_month  INTEGER NOT NULL,"
        "  due_year   INTEGER NOT NULL,"
        "  FOREIGN KEY (user_id) REFERENCES users(id)"
        ");"

        "CREATE TABLE IF NOT EXISTS budget_limits ("
        "  id           INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  user_id      INTEGER NOT NULL,"
        "  category     TEXT    NOT NULL,"
        "  limit_amount REAL    NOT NULL,"
        "  month        INTEGER NOT NULL,"
        "  year         INTEGER NOT NULL,"
        "  UNIQUE(user_id, category, month, year),"
        "  FOREIGN KEY (user_id) REFERENCES users(id)"
        ");";

    char* errMsg = nullptr;
    if (sqlite3_exec(db_, sql, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::string msg = errMsg ? errMsg : "unknown error";
        sqlite3_free(errMsg);
        throw std::runtime_error("Schema init failed: " + msg);
    }
}

std::string Database::hashPassword(const std::string& password) {
    unsigned char hash[CC_SHA256_DIGEST_LENGTH];
    CC_SHA256(password.c_str(), static_cast<CC_LONG>(password.length()), hash);
    char hex[CC_SHA256_DIGEST_LENGTH * 2 + 1];
    for (int i = 0; i < CC_SHA256_DIGEST_LENGTH; ++i) {
        std::snprintf(hex + i * 2, 3, "%02x", hash[i]);
    }
    return std::string(hex);
}

int Database::registerUser(const std::string& username, const std::string& password) {
    const char* sql = "INSERT INTO users (username, password_hash) VALUES (?, ?)";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return -1;

    std::string hash = hashPassword(password);
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, hash.c_str(),     -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc == SQLITE_DONE) {
        return static_cast<int>(sqlite3_last_insert_rowid(db_));
    }
    return -1; // SQLITE_CONSTRAINT means username taken
}

int Database::loginUser(const std::string& username, const std::string& password) {
    const char* sql = "SELECT id, password_hash FROM users WHERE username = ?";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return -1;

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);

    int userId = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        int         id          = sqlite3_column_int (stmt, 0);
        const char* storedHash  = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        if (storedHash && hashPassword(password) == storedHash) {
            userId = id;
        }
    }
    sqlite3_finalize(stmt);
    return userId;
}

std::vector<Expense> Database::loadExpenses(int userId) {
    const char* sql =
        "SELECT category, amount, day, month, year, description "
        "FROM expenses WHERE user_id = ?";
    sqlite3_stmt* stmt = nullptr;
    std::vector<Expense> result;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return result;

    sqlite3_bind_int(stmt, 1, userId);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Expense e;
        e.category    = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        e.amount      = sqlite3_column_double(stmt, 1);
        e.date.day    = sqlite3_column_int   (stmt, 2);
        e.date.month  = sqlite3_column_int   (stmt, 3);
        e.date.year   = sqlite3_column_int   (stmt, 4);
        const char* desc = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        e.description = desc ? desc : "";
        result.push_back(e);
    }
    sqlite3_finalize(stmt);
    return result;
}

std::vector<Bill> Database::loadBills(int userId) {
    const char* sql =
        "SELECT name, amount_due, due_day, due_month, due_year "
        "FROM bills WHERE user_id = ?";
    sqlite3_stmt* stmt = nullptr;
    std::vector<Bill> result;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return result;

    sqlite3_bind_int(stmt, 1, userId);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Bill b;
        b.name            = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        b.amountDue       = sqlite3_column_double(stmt, 1);
        b.dueDate.day     = sqlite3_column_int   (stmt, 2);
        b.dueDate.month   = sqlite3_column_int   (stmt, 3);
        b.dueDate.year    = sqlite3_column_int   (stmt, 4);
        b.isPaid          = false;
        result.push_back(b);
    }
    sqlite3_finalize(stmt);
    return result;
}

std::vector<Database::BudgetLimitRow> Database::loadBudgetLimits(int userId) {
    const char* sql =
        "SELECT category, limit_amount, month, year "
        "FROM budget_limits WHERE user_id = ?";
    sqlite3_stmt* stmt = nullptr;
    std::vector<BudgetLimitRow> result;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return result;

    sqlite3_bind_int(stmt, 1, userId);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        BudgetLimitRow row;
        row.category = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        row.limit    = sqlite3_column_double(stmt, 1);
        row.month    = sqlite3_column_int   (stmt, 2);
        row.year     = sqlite3_column_int   (stmt, 3);
        result.push_back(row);
    }
    sqlite3_finalize(stmt);
    return result;
}

void Database::insertExpense(int userId, const Expense& e) {
    const char* sql =
        "INSERT INTO expenses (user_id, category, amount, day, month, year, description) "
        "VALUES (?, ?, ?, ?, ?, ?, ?)";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return;

    sqlite3_bind_int   (stmt, 1, userId);
    sqlite3_bind_text  (stmt, 2, e.category.c_str(),    -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 3, e.amount);
    sqlite3_bind_int   (stmt, 4, e.date.day);
    sqlite3_bind_int   (stmt, 5, e.date.month);
    sqlite3_bind_int   (stmt, 6, e.date.year);
    sqlite3_bind_text  (stmt, 7, e.description.c_str(), -1, SQLITE_TRANSIENT);

    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void Database::insertBill(int userId, const Bill& b) {
    const char* sql =
        "INSERT INTO bills (user_id, name, amount_due, due_day, due_month, due_year) "
        "VALUES (?, ?, ?, ?, ?, ?)";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return;

    sqlite3_bind_int   (stmt, 1, userId);
    sqlite3_bind_text  (stmt, 2, b.name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 3, b.amountDue);
    sqlite3_bind_int   (stmt, 4, b.dueDate.day);
    sqlite3_bind_int   (stmt, 5, b.dueDate.month);
    sqlite3_bind_int   (stmt, 6, b.dueDate.year);

    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void Database::upsertBudgetLimit(int userId, const std::string& category,
                                  double limit, int month, int year) {
    const char* sql =
        "INSERT INTO budget_limits (user_id, category, limit_amount, month, year) "
        "VALUES (?, ?, ?, ?, ?) "
        "ON CONFLICT(user_id, category, month, year) "
        "DO UPDATE SET limit_amount = excluded.limit_amount";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return;

    sqlite3_bind_int   (stmt, 1, userId);
    sqlite3_bind_text  (stmt, 2, category.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 3, limit);
    sqlite3_bind_int   (stmt, 4, month);
    sqlite3_bind_int   (stmt, 5, year);

    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void Database::deleteBill(int userId, const std::string& name) {
    const char* sql = "DELETE FROM bills WHERE user_id = ? AND name = ?";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return;

    sqlite3_bind_int (stmt, 1, userId);
    sqlite3_bind_text(stmt, 2, name.c_str(), -1, SQLITE_TRANSIENT);

    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}
