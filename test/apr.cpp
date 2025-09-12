#include <apr.h>
#include <apr_pools.h>
#include <apr_strings.h>
#include <apr_time.h>
#include <gtest/gtest.h>

class AprTest : public ::testing::Test
{
protected:
  apr_pool_t* pool;

  void SetUp() override
  {
    apr_initialize();
    apr_pool_create(&pool, NULL);
  }

  void TearDown() override
  {
    if (pool)
    {
      apr_pool_destroy(pool);
      pool = NULL;
    }
    apr_terminate();
  }
};

TEST_F(AprTest, TestPoolCreation)
{
  ASSERT_NE(pool, nullptr) << "Failed to create APR memory pool";
}

TEST_F(AprTest, TestStringFormatting)
{
  const char* name = "test";
  int value = 42;
  char* result;

  apr_pool_t* subpool;
  apr_pool_create(&subpool, pool);

  result = apr_psprintf(subpool, "%s_%d", name, value);
  EXPECT_STREQ(result, "test_42") << "String formatting failed";

  apr_pool_destroy(subpool);
}

TEST_F(AprTest, TestTimeFunctions)
{
  apr_time_t now = apr_time_now();
  EXPECT_GT(now, 0) << "Invalid current time";

  apr_time_exp_t exp_time;
  apr_status_t rv = apr_time_exp_lt(&exp_time, now);

  EXPECT_EQ(rv, APR_SUCCESS) << "Failed to expand time";
  EXPECT_GE(exp_time.tm_year, 100) << "Invalid year"; // Years since 1900
  EXPECT_GE(exp_time.tm_mon, 0) << "Invalid month";
  EXPECT_LE(exp_time.tm_mon, 11) << "Invalid month";
  EXPECT_GE(exp_time.tm_mday, 1) << "Invalid day";
  EXPECT_LE(exp_time.tm_mday, 31) << "Invalid day";
}

TEST_F(AprTest, TestMemoryAllocation)
{
  const int count = 10;
  int* numbers = (int*)apr_palloc(pool, count * sizeof(int));

  ASSERT_NE(numbers, nullptr) << "Failed to allocate memory";

  // Test that we can write to and read from the allocated memory
  for (int i = 0; i < count; ++i)
  {
    numbers[i] = i * i;
  }

  for (int i = 0; i < count; ++i)
  {
    EXPECT_EQ(numbers[i], i * i)
      << "Memory allocation/access failed at index " << i;
  }
}

TEST_F(AprTest, TestStringComparison)
{
  const char* str1 = "Hello";
  const char* str2 = "HELLO";
  const char* str3 = "Hello";

  // Case-sensitive comparison
  EXPECT_EQ(apr_strnatcmp(str1, str3), 0)
    << "Identical strings should compare equal";
  EXPECT_NE(apr_strnatcmp(str1, str2), 0)
    << "Differently cased strings should not compare equal";

  // Case-insensitive comparison
  EXPECT_EQ(apr_strnatcasecmp(str1, str2), 0)
    << "Case-insensitive comparison failed";
}
