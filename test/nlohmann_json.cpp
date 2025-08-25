#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class JsonTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    // Test data
    simple_object = {{"pi", 3.141},
                     {"happy", true},
                     {"name", "Niels"},
                     {"nothing", nullptr},
                     {"answer", {{"everything", 42}}},
                     {"list", {1, 0, 2}},
                     {"object", {{"currency", "USD"}, {"value", 42.99}}}};
  }

  json simple_object;
};

// Test basic JSON construction and access
TEST_F(JsonTest, BasicOperations)
{
  // Test object access
  EXPECT_DOUBLE_EQ(simple_object["pi"], 3.141);
  EXPECT_TRUE(simple_object["happy"]);
  EXPECT_EQ(simple_object["name"], "Niels");
  EXPECT_TRUE(simple_object["nothing"].is_null());

  // Test nested object access
  EXPECT_EQ(simple_object["answer"]["everything"], 42);

  // Test array access
  EXPECT_EQ(simple_object["list"][0], 1);
  EXPECT_EQ(simple_object["list"][1], 0);
  EXPECT_EQ(simple_object["list"][2], 2);

  // Test object within object
  EXPECT_EQ(simple_object["object"]["currency"], "USD");
  EXPECT_DOUBLE_EQ(simple_object["object"]["value"], 42.99);
}

// Test JSON parsing from string
TEST_F(JsonTest, ParseFromString)
{
  const char* json_string = R"(
        {
            "name": "John Doe",
            "age": 30,
            "scores": [95, 87, 92]
        }
    )";

  auto j = json::parse(json_string);

  EXPECT_EQ(j["name"], "John Doe");
  EXPECT_EQ(j["age"], 30);
  EXPECT_EQ(j["scores"].size(), 3);
  EXPECT_EQ(j["scores"][0], 95);
  EXPECT_EQ(j["scores"][1], 87);
  EXPECT_EQ(j["scores"][2], 92);
}

// Test JSON serialization
TEST_F(JsonTest, SerializeToString)
{
  json j = {{"name", "John Doe"}, {"age", 30}, {"scores", {95, 87, 92}}};

  std::string serialized = j.dump();
  // Basic checks for serialized string
  EXPECT_NE(serialized.find("John Doe"), std::string::npos);
  EXPECT_NE(serialized.find("30"), std::string::npos);

  // Parse it back and verify
  auto parsed = json::parse(serialized);
  EXPECT_EQ(parsed["name"], "John Doe");
  EXPECT_EQ(parsed["age"], 30);
}

// Test iteration
TEST_F(JsonTest, Iteration)
{
  json j = {{"one", 1}, {"two", 2}, {"three", 3}};

  int sum = 0;
  for (auto& [key, value] : j.items())
  {
    sum += value.get<int>();
  }

  EXPECT_EQ(sum, 6);
}

// Test type checking
TEST_F(JsonTest, TypeChecking)
{
  json j = {{"null", nullptr},
            {"boolean", true},
            {"number", 42},
            {"float", 3.14},
            {"string", "hello"},
            {"array", {1, 2, 3}},
            {"object", {{"key", "value"}}}};

  EXPECT_TRUE(j["null"].is_null());
  EXPECT_TRUE(j["boolean"].is_boolean());
  EXPECT_TRUE(j["number"].is_number_integer());
  EXPECT_TRUE(j["float"].is_number_float());
  EXPECT_TRUE(j["string"].is_string());
  EXPECT_TRUE(j["array"].is_array());
  EXPECT_TRUE(j["object"].is_object());
}

// Test exception handling
TEST_F(JsonTest, ExceptionHandling)
{
  // Test parse error
  EXPECT_THROW({ json j = json::parse("{invalid json}"); }, json::parse_error);

  // Test out of range access - nlohmann_json returns null for out of range
  // access
  json j = {1, 2, 3};
  EXPECT_NO_THROW({
    auto x = j[3];
    EXPECT_TRUE(x.is_null());
  });
}
