#include <filesystem>
#include <iostream>
#include <string>

#include <gtest/gtest.h>
#include <sqlite3.h>

// must be included after sqlite3.h
#include <spatialite.h>

namespace
{

  class LibSpatialiteTest : public ::testing::Test
  {
  protected:
    static void SetUpTestSuite()
    {
      // Initialize SQLite and SpatiaLite
      sqlite3_initialize();
      spatialite_init(0);
    }

    static void TearDownTestSuite()
    {
      // Cleanup
      spatialite_cleanup();
      sqlite3_shutdown();
    }

    void SetUp() override
    {
      // Create an in-memory database for testing
      int rc = sqlite3_open_v2(
        ":memory:",
        &db_,
        SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_URI,
        nullptr);
      if (rc != SQLITE_OK)
      {
        std::cerr << "Failed to open database: " << sqlite3_errmsg(db_)
                  << std::endl;
        sqlite3_close(db_);
        GTEST_SKIP() << "Failed to open SQLite database";
      }

      // Enable loading extensions
      sqlite3_enable_load_extension(db_, 1);

      // Initialize SpatiaLite
      char* errMsg = nullptr;
      rc = sqlite3_exec(
        db_, "SELECT InitSpatialMetaData(1)", nullptr, nullptr, &errMsg);
      if (rc != SQLITE_OK)
      {
        std::cerr << "Failed to initialize SpatiaLite: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        sqlite3_close(db_);
        GTEST_SKIP() << "Failed to initialize SpatiaLite";
      }
    }

    void TearDown() override
    {
      if (db_)
      {
        sqlite3_close(db_);
        db_ = nullptr;
      }
    }

    sqlite3* db_ = nullptr;
  };

  TEST_F(LibSpatialiteTest, TestSpatialMetaData)
  {
    // Check if spatial_ref_sys table exists
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT name FROM sqlite_master WHERE type='table' AND "
                      "name='spatial_ref_sys'";

    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    ASSERT_EQ(rc, SQLITE_OK) << "Failed to prepare SQL statement";

    rc = sqlite3_step(stmt);
    ASSERT_EQ(rc, SQLITE_ROW) << "spatial_ref_sys table not found";

    const unsigned char* tableName = sqlite3_column_text(stmt, 0);
    ASSERT_NE(tableName, nullptr);
    ASSERT_STREQ(reinterpret_cast<const char*>(tableName), "spatial_ref_sys");

    sqlite3_finalize(stmt);
  }

  TEST_F(LibSpatialiteTest, TestCreateSpatialTable)
  {
    // Create a simple spatial table
    const char* sql =
      "CREATE TABLE test_geometries ("
      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
      "name TEXT NOT NULL"
      ");"
      "SELECT AddGeometryColumn('test_geometries', 'geometry', 4326, 'POINT', "
      "'XY');"
      "SELECT CreateSpatialIndex('test_geometries', 'geometry');";

    char* errMsg = nullptr;
    int rc = sqlite3_exec(db_, sql, nullptr, nullptr, &errMsg);
    ASSERT_EQ(rc, SQLITE_OK)
      << "Failed to create spatial table: " << (errMsg ? errMsg : "");
    if (errMsg)
      sqlite3_free(errMsg);

    // Verify the table was created
    sqlite3_stmt* stmt = nullptr;
    const char* verifySql = "SELECT name FROM sqlite_master WHERE type='table' "
                            "AND name='test_geometries'";

    rc = sqlite3_prepare_v2(db_, verifySql, -1, &stmt, nullptr);
    ASSERT_EQ(rc, SQLITE_OK) << "Failed to prepare verification SQL";

    rc = sqlite3_step(stmt);
    ASSERT_EQ(rc, SQLITE_ROW) << "test_geometries table not found";

    sqlite3_finalize(stmt);
  }

  TEST_F(LibSpatialiteTest, TestSpatialQuery)
  {
    // Create a test table and insert a point
    const char* setupSql =
      "CREATE TABLE places ("
      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
      "name TEXT NOT NULL"
      ");"
      "SELECT AddGeometryColumn('places', 'location', 4326, 'POINT', 'XY');"
      "INSERT INTO places (name, location) VALUES "
      "('Location 1', GeomFromText('POINT(12.4923 41.8902)', 4326)),"
      "('Location 2', GeomFromText('POINT(12.4534 41.9028)', 4326));";

    char* errMsg = nullptr;
    int rc = sqlite3_exec(db_, setupSql, nullptr, nullptr, &errMsg);
    ASSERT_EQ(rc, SQLITE_OK)
      << "Failed to set up test data: " << (errMsg ? errMsg : "");
    if (errMsg)
      sqlite3_free(errMsg);

    // Perform a spatial query
    sqlite3_stmt* stmt = nullptr;
    const char* querySql =
      "SELECT name, AsText(location) "
      "FROM places "
      "WHERE Distance(location, MakePoint(12.4964, 41.9028, 4326)) < 10000";

    rc = sqlite3_prepare_v2(db_, querySql, -1, &stmt, nullptr);
    ASSERT_EQ(rc, SQLITE_OK) << "Failed to prepare spatial query";

    // Execute the query and check results
    bool foundLocation2 = false;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
      const unsigned char* name = sqlite3_column_text(stmt, 0);
      const unsigned char* wkt = sqlite3_column_text(stmt, 1);

      if (name &&
          strcmp(reinterpret_cast<const char*>(name), "Location 2") == 0)
      {
        foundLocation2 = true;
        ASSERT_NE(wkt, nullptr);
        // Verify the WKT is as expected
        std::string wktStr(reinterpret_cast<const char*>(wkt));
        ASSERT_NE(wktStr.find("POINT(12.4534 41.9028)"), std::string::npos);
      }
    }

    ASSERT_EQ(rc, SQLITE_DONE)
      << "Error during query execution: " << sqlite3_errmsg(db_);
    ASSERT_TRUE(foundLocation2)
      << "Expected to find Location 2 in query results";

    sqlite3_finalize(stmt);
  }

} // namespace
