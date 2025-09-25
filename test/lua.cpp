#include <string>
#include <vector>

#include <gtest/gtest.h>

extern "C"
{
#include <lua/lauxlib.h>
#include <lua/lua.h>
#include <lua/lualib.h>
}

class LuaTest : public ::testing::Test
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

TEST_F(LuaTest, VersionCheck)
{
  // Just verify that we can get the Lua version
  lua_pushstring(L, LUA_VERSION);
  EXPECT_TRUE(lua_isstring(L, -1));

  const char* version = lua_tostring(L, -1);
  std::cout << "Lua version: " << version << std::endl;

  EXPECT_TRUE(version != nullptr);
  EXPECT_TRUE(strlen(version) > 0);

  lua_pop(L, 1);
}

TEST_F(LuaTest, BasicExecution)
{
  // Execute a simple Lua script
  const char* script = "return 42";

  int status = luaL_loadstring(L, script);
  ASSERT_EQ(status, LUA_OK) << "Failed to load Lua script";

  status = lua_pcall(L, 0, 1, 0);
  ASSERT_EQ(status, LUA_OK) << "Failed to execute Lua script";

  // Check return value
  EXPECT_TRUE(lua_isnumber(L, -1));
  EXPECT_EQ(lua_tonumber(L, -1), 42);

  lua_pop(L, 1);
}
