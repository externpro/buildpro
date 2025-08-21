#include <chrono>
#include <string>
#include <vector>

#include <fmt/chrono.h>
#include <fmt/color.h>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <gtest/gtest.h>

// Custom type for testing
struct Point
{
  double x, y;
};

// Formatter specialization for Point
template <>
struct fmt::formatter<Point>
{
  constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

  template <typename FormatContext>
  auto format(const Point& p, FormatContext& ctx) const
  {
    return format_to(ctx.out(), "({:.1f}, {:.1f})", p.x, p.y);
  }
};

class FmtTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    // Any setup code would go here
  }
};

TEST_F(FmtTest, BasicFormatting)
{
  std::string hello = fmt::format("Hello, {}!", "world");
  EXPECT_EQ(hello, "Hello, world!");
}

TEST_F(FmtTest, NumberFormatting)
{
  std::string num = fmt::format("The answer is {:.2f}", 42.12345);
  EXPECT_EQ(num, "The answer is 42.12");
}

TEST_F(FmtTest, PositionalArguments)
{
  std::string pos = fmt::format("{1} {0}!", "better", "make");
  EXPECT_EQ(pos, "make better!");
}

TEST_F(FmtTest, NamedArguments)
{
  std::string named = fmt::format("I'd rather be {verb} {where}!",
                                  fmt::arg("verb", "sailing"),
                                  fmt::arg("where", "away"));
  EXPECT_EQ(named, "I'd rather be sailing away!");
}

TEST_F(FmtTest, ContainerFormatting)
{
  std::vector<int> v = {1, 2, 3, 5, 8, 13};
  std::string vec_str = fmt::format("Fibonacci: {}", v);
  EXPECT_EQ(vec_str, "Fibonacci: [1, 2, 3, 5, 8, 13]");
}

TEST_F(FmtTest, DateTimeFormatting)
{
  using namespace std::chrono;
  // Use a fixed time point for consistent testing
  auto time_point =
    system_clock::time_point{} + std::chrono::hours(24 * 365 * 50);
  std::string time_str = fmt::format("Time: {:%Y-%m-%d %H:%M}", time_point);

  // Check the exact expected format with fixed time
  EXPECT_EQ(time_str, "Time: 2019-12-20 00:00");
}

TEST_F(FmtTest, CustomTypeFormatting)
{
  Point p{3.14, 2.71};
  std::string point_str = fmt::format("Point: {}", p);
  EXPECT_EQ(point_str, "Point: (3.1, 2.7)");
}

TEST_F(FmtTest, MemoryBuffer)
{
  fmt::memory_buffer buf;
  fmt::format_to(std::back_inserter(buf), "The answer is {}", 42);
  std::string buf_str = fmt::to_string(buf);
  EXPECT_EQ(buf_str, "The answer is 42");
}

TEST_F(FmtTest, TextStyling)
{
  std::string styled = fmt::format(
    fg(fmt::color::crimson) | fmt::emphasis::bold, "Hello, {}!", "world");
  EXPECT_NE(styled.find("Hello, world!"), std::string::npos);
}

#if defined(__cpp_lib_format) && __cpp_lib_format >= 201907L
#if FMT_VERSION >= 80000
#if FMT_USE_CONSTEXPR
TEST_F(FmtTest, CompileTimeFormatting)
{
  constexpr auto const_str = FMT_COMPILE("The answer is {}\n");
  std::string compiled = fmt::format(const_str, 42);
  EXPECT_EQ(compiled, "The answer is 42\n");
}
#endif
#endif
#endif
