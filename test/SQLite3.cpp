#include <string>
#include <vector>

#include <gtest/gtest.h>
#include <sqlite3.h>

class SQLite3Test : public ::testing::Test
{
protected:
  sqlite3* db = nullptr;
  char* errMsg = nullptr;

  void SetUp() override
  {
    // Open an in-memory database
    int rc = sqlite3_open(":memory:", &db);
    ASSERT_EQ(SQLITE_OK, rc)
      << "Failed to open database: " << sqlite3_errmsg(db);

    // Enable foreign key constraints
    rc =
      sqlite3_exec(db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, &errMsg);
    ASSERT_EQ(SQLITE_OK, rc) << "Failed to enable foreign keys: " << errMsg;
  }

  void TearDown() override
  {
    if (db)
    {
      sqlite3_close(db);
      db = nullptr;
    }
    if (errMsg)
    {
      sqlite3_free(errMsg);
      errMsg = nullptr;
    }
  }

  int execute(const std::string& sql)
  {
    if (errMsg)
    {
      sqlite3_free(errMsg);
      errMsg = nullptr;
    }
    return sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);
  }

  struct QueryResult
  {
    std::vector<std::vector<std::string>> rows;
    std::vector<std::string> columns;
  };

  QueryResult query(const std::string& sql)
  {
    QueryResult result;
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
      throw std::runtime_error("Failed to prepare statement: " +
                               std::string(sqlite3_errmsg(db)));
    }

    // Get column names
    int colCount = sqlite3_column_count(stmt);
    for (int i = 0; i < colCount; ++i)
    {
      result.columns.push_back(sqlite3_column_name(stmt, i));
    }

    // Get rows
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
      std::vector<std::string> row;
      for (int i = 0; i < colCount; ++i)
      {
        const char* val =
          reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
        row.push_back(val ? val : "");
      }
      result.rows.push_back(row);
    }

    if (rc != SQLITE_DONE)
    {
      sqlite3_finalize(stmt);
      throw std::runtime_error("Error executing query: " +
                               std::string(sqlite3_errmsg(db)));
    }

    sqlite3_finalize(stmt);
    return result;
  }
};

TEST_F(SQLite3Test, TestBasicOperations)
{
  // Create a table
  const char* createTableSql = R"(
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            email TEXT UNIQUE NOT NULL,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        );
    )";

  int rc = execute(createTableSql);
  ASSERT_EQ(SQLITE_OK, rc) << "Failed to create table: "
                           << (errMsg ? errMsg : "");

  // Insert data
  const char* insertSql = R"(
        INSERT INTO users (name, email) VALUES 
        ('John Doe', 'john@example.com'),
        ('Jane Smith', 'jane@example.com');
    )";

  rc = execute(insertSql);
  ASSERT_EQ(SQLITE_OK, rc) << "Failed to insert data: "
                           << (errMsg ? errMsg : "");

  // Query data
  auto result = query("SELECT id, name, email FROM users ORDER BY id;");

  // Verify columns
  ASSERT_EQ(3, result.columns.size());
  EXPECT_EQ("id", result.columns[0]);
  EXPECT_EQ("name", result.columns[1]);
  EXPECT_EQ("email", result.columns[2]);

  // Verify data
  ASSERT_EQ(2, result.rows.size());

  EXPECT_EQ("1", result.rows[0][0]);
  EXPECT_EQ("John Doe", result.rows[0][1]);
  EXPECT_EQ("john@example.com", result.rows[0][2]);

  EXPECT_EQ("2", result.rows[1][0]);
  EXPECT_EQ("Jane Smith", result.rows[1][1]);
  EXPECT_EQ("jane@example.com", result.rows[1][2]);
}

TEST_F(SQLite3Test, TestPreparedStatements)
{
  // Create a table
  int rc = execute(R"(
        CREATE TABLE products (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            price REAL NOT NULL
        )
    )");
  ASSERT_EQ(SQLITE_OK, rc) << "Failed to create table: "
                           << (errMsg ? errMsg : "");

  // Insert data using prepared statement
  sqlite3_stmt* stmt = nullptr;
  const char* insertSql = "INSERT INTO products (name, price) VALUES (?, ?)";

  rc = sqlite3_prepare_v2(db, insertSql, -1, &stmt, nullptr);
  ASSERT_EQ(SQLITE_OK, rc) << "Failed to prepare statement: "
                           << sqlite3_errmsg(db);

  // Insert first product
  sqlite3_bind_text(stmt, 1, "Laptop", -1, SQLITE_STATIC);
  sqlite3_bind_double(stmt, 2, 999.99);

  rc = sqlite3_step(stmt);
  ASSERT_EQ(SQLITE_DONE, rc)
    << "Failed to execute statement: " << sqlite3_errmsg(db);

  // Reset statement for reuse
  sqlite3_reset(stmt);

  // Insert second product
  sqlite3_bind_text(stmt, 1, "Mouse", -1, SQLITE_STATIC);
  sqlite3_bind_double(stmt, 2, 29.99);

  rc = sqlite3_step(stmt);
  ASSERT_EQ(SQLITE_DONE, rc)
    << "Failed to execute statement: " << sqlite3_errmsg(db);

  sqlite3_finalize(stmt);

  // Query and verify
  auto result = query("SELECT name, price FROM products ORDER BY price;");

  ASSERT_EQ(2, result.rows.size());
  EXPECT_EQ("Mouse", result.rows[0][0]);
  EXPECT_EQ("29.99", result.rows[0][1]);
  EXPECT_EQ("Laptop", result.rows[1][0]);
  EXPECT_EQ("999.99", result.rows[1][1]);
}

TEST_F(SQLite3Test, TestTransactions)
{
  // Create a table
  int rc = execute(
    "CREATE TABLE accounts (id INTEGER PRIMARY KEY, balance REAL NOT NULL);");
  ASSERT_EQ(SQLITE_OK, rc) << "Failed to create table: "
                           << (errMsg ? errMsg : "");

  // Insert initial data
  rc = execute(
    "INSERT INTO accounts (id, balance) VALUES (1, 1000.0), (2, 500.0);");
  ASSERT_EQ(SQLITE_OK, rc) << "Failed to insert data: "
                           << (errMsg ? errMsg : "");

  // Start transaction
  rc = execute("BEGIN TRANSACTION;");
  ASSERT_EQ(SQLITE_OK, rc) << "Failed to begin transaction: "
                           << (errMsg ? errMsg : "");

  try
  {
    // Transfer money
    rc = execute("UPDATE accounts SET balance = balance - 200.0 WHERE id = 1;");
    if (rc != SQLITE_OK)
      throw std::runtime_error("Failed to update sender balance");

    rc = execute("UPDATE accounts SET balance = balance + 200.0 WHERE id = 2;");
    if (rc != SQLITE_OK)
      throw std::runtime_error("Failed to update recipient balance");

    // Commit transaction
    rc = execute("COMMIT;");
    ASSERT_EQ(SQLITE_OK, rc)
      << "Failed to commit transaction: " << (errMsg ? errMsg : "");
  }
  catch (...)
  {
    execute("ROLLBACK;");
    throw;
  }

  // Verify balances
  auto result = query("SELECT id, balance FROM accounts ORDER BY id;");

  ASSERT_EQ(2, result.rows.size());
  EXPECT_EQ("1", result.rows[0][0]);
  EXPECT_EQ("800.0", result.rows[0][1]);
  EXPECT_EQ("2", result.rows[1][0]);
  EXPECT_EQ("700.0", result.rows[1][1]);
}
