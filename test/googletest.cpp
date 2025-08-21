#include <memory>
#include <string>
#include <vector>

#include <gtest/gtest.h>

// Simple function to test
int Add(int a, int b)
{
  return a + b;
}

// A simple class to test
class Counter
{
public:
  Counter() : count_(0) { }
  void Increment() { ++count_; }
  void Reset() { count_ = 0; }
  int GetCount() const { return count_; }

private:
  int count_;
};

// Basic test case
TEST(BasicTest, SimpleAddition)
{
  EXPECT_EQ(2 + 2, 4);
  EXPECT_NE(2 + 2, 5);
  EXPECT_LT(1, 10);
  EXPECT_LE(2, 2);
  EXPECT_GT(10, 1);
  EXPECT_GE(5, 5);
}

// Test case for the Add function
TEST(FunctionTest, TestAdd)
{
  EXPECT_EQ(Add(1, 1), 2);
  EXPECT_EQ(Add(-1, 1), 0);
  EXPECT_EQ(Add(0, 0), 0);
  EXPECT_EQ(Add(100, 200), 300);
}

// Test case for the Counter class
class CounterTest : public ::testing::Test
{
protected:
  void SetUp() override { counter.reset(new Counter()); }

  void TearDown() override { counter.reset(); }

  std::unique_ptr<Counter> counter;
};

TEST_F(CounterTest, StartsAtZero)
{
  EXPECT_EQ(counter->GetCount(), 0);
}

TEST_F(CounterTest, IncrementIncreasesCount)
{
  counter->Increment();
  EXPECT_EQ(counter->GetCount(), 1);

  counter->Increment();
  EXPECT_EQ(counter->GetCount(), 2);
}

TEST_F(CounterTest, ResetSetsCountToZero)
{
  counter->Increment();
  counter->Increment();
  counter->Reset();
  EXPECT_EQ(counter->GetCount(), 0);
}

// Test case with assertions
TEST(AssertionTest, VariousAssertions)
{
  // Boolean conditions
  ASSERT_TRUE(true);
  ASSERT_FALSE(false);

  // String comparison
  std::string str1 = "hello";
  std::string str2 = "world";
  ASSERT_STREQ("hello", str1.c_str());
  ASSERT_STRNE(str1.c_str(), str2.c_str());

  // Floating point comparison
  ASSERT_FLOAT_EQ(1.0f, 1.0f);
  ASSERT_DOUBLE_EQ(1.0, 1.0);
  ASSERT_NEAR(1.0, 1.1, 0.2); // Within 0.2
}

// Test case with parameterized tests
class ParameterizedTest : public ::testing::TestWithParam<int>
{
};

TEST_P(ParameterizedTest, IsEven)
{
  int n = GetParam();
  EXPECT_EQ(n % 2, 0);
}

INSTANTIATE_TEST_SUITE_P(EvenNumbers,
                         ParameterizedTest,
                         testing::Values(2, 4, 6, 8, 10));

// Test case with typed tests
template <typename T>
class TypedTest : public ::testing::Test
{
};

typedef testing::Types<int, float, double> NumericTypes;
TYPED_TEST_SUITE(TypedTest, NumericTypes);

TYPED_TEST(TypedTest, DefaultConstructorIsZero)
{
  TypeParam n{};
  EXPECT_EQ(n, 0);
}
