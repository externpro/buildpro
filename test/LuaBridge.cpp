#include <cmath>
#include <string>

#include <gtest/gtest.h>

extern "C"
{
#include <lua/lauxlib.h>
#include <lua/lua.h>
#include <lua/lualib.h>
}

#include <LuaBridge/LuaBridge.h>

class LuaBridgeTest : public ::testing::Test
{
protected:
  lua_State* L = nullptr;

  void SetUp() override
  {
    // Create a new Lua state
    L = luaL_newstate();
    ASSERT_NE(L, nullptr) << "Failed to create Lua state";

    // Open standard libraries
    luaL_openlibs(L);
  }

  void TearDown() override
  {
    if (L)
    {
      lua_close(L);
      L = nullptr;
    }
  }
};

namespace
{

  int add(int a, int b)
  {
    return a + b;
  }

  struct Point
  {
    Point(double x_, double y_) : x(x_), y(y_) { }

    double length() const { return std::sqrt(x * x + y * y); }

    double x = 0.0;
    double y = 0.0;
  };

  std::string greet(const std::string& name)
  {
    return std::string("Hello, ") + name;
  }

} // namespace

TEST_F(LuaBridgeTest, CanCallBoundFunction)
{
  using namespace luabridge;

  getGlobalNamespace(L).addFunction("add", &add);

  const char* script = "return add(2, 3)";

  int status = luaL_loadstring(L, script);
  ASSERT_EQ(status, LUA_OK) << "Failed to load Lua script";

  status = lua_pcall(L, 0, 1, 0);
  ASSERT_EQ(status, LUA_OK) << "Failed to execute Lua script";

  ASSERT_TRUE(lua_isnumber(L, -1));
  int result = static_cast<int>(lua_tointeger(L, -1));
  EXPECT_EQ(result, 5);

  lua_pop(L, 1);
}

TEST_F(LuaBridgeTest, CanUseBoundClass)
{
  using namespace luabridge;

  getGlobalNamespace(L)
    .beginClass<Point>("Point")
    .addConstructor<void (*)(double, double)>()
    .addData("x", &Point::x)
    .addData("y", &Point::y)
    .addFunction("length", &Point::length)
    .endClass();

  const char* script = "p = Point(3.0, 4.0)\n"
                       "return p:length(), p.x, p.y";

  int status = luaL_loadstring(L, script);
  ASSERT_EQ(status, LUA_OK) << "Failed to load Lua script";

  status = lua_pcall(L, 0, 3, 0);
  ASSERT_EQ(status, LUA_OK) << "Failed to execute Lua script";

  ASSERT_TRUE(lua_isnumber(L, -3));
  ASSERT_TRUE(lua_isnumber(L, -2));
  ASSERT_TRUE(lua_isnumber(L, -1));

  double len = lua_tonumber(L, -3);
  double x = lua_tonumber(L, -2);
  double y = lua_tonumber(L, -1);

  EXPECT_DOUBLE_EQ(len, 5.0);
  EXPECT_DOUBLE_EQ(x, 3.0);
  EXPECT_DOUBLE_EQ(y, 4.0);

  lua_pop(L, 3);
}

TEST_F(LuaBridgeTest, CanUseNamespaceAndStdString)
{
  using namespace luabridge;

  getGlobalNamespace(L)
    .beginNamespace("test")
    .addFunction("greet", &greet)
    .endNamespace();

  const char* script = "return test.greet('LuaBridge')";

  int status = luaL_loadstring(L, script);
  ASSERT_EQ(status, LUA_OK) << "Failed to load Lua script";

  status = lua_pcall(L, 0, 1, 0);
  ASSERT_EQ(status, LUA_OK) << "Failed to execute Lua script";

  luabridge::LuaRef result = luabridge::LuaRef::fromStack(L, -1);
  ASSERT_TRUE(result.isString());

  std::string str = result.cast<std::string>();
  EXPECT_EQ(str, std::string("Hello, LuaBridge"));

  lua_pop(L, 1);
}
