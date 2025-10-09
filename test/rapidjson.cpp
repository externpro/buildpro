#include <iostream>
#include <string>
#include <vector>

#include <gtest/gtest.h>
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/schema.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

using namespace rapidjson;

class RapidJsonTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    // Test data
    simpleJson =
      "{\"project\":\"rapidjson\",\"stars\":10,\"author\":\"Tencent\"}";
    arrayJson = "[1, 2, 3, 4, 5]";
    nestedJson = R"(
            {
                "name": "RapidJSON",
                "version": "1.1.0",
                "features": ["fast", "portable", "DOM", "SAX"],
                "stats": {
                    "stars": 12000,
                    "forks": 3000
                },
                "examples": [
                    {"name": "tutorial", "type": "basic"},
                    {"name": "pretty", "type": "writer"}
                ]
            }
        )";
  }

  // Test data
  const char* simpleJson;
  const char* arrayJson;
  const char* nestedJson;
};

// Test basic JSON parsing
TEST_F(RapidJsonTest, BasicParsing)
{
  Document doc;
  ASSERT_FALSE(doc.Parse(simpleJson).HasParseError());

  // Check document is an object
  EXPECT_TRUE(doc.IsObject());

  // Check values
  EXPECT_TRUE(doc.HasMember("project"));
  EXPECT_TRUE(doc.HasMember("stars"));
  EXPECT_TRUE(doc.HasMember("author"));

  EXPECT_TRUE(doc["project"].IsString());
  EXPECT_STREQ(doc["project"].GetString(), "rapidjson");

  EXPECT_TRUE(doc["stars"].IsInt());
  EXPECT_EQ(doc["stars"].GetInt(), 10);

  EXPECT_TRUE(doc["author"].IsString());
  EXPECT_STREQ(doc["author"].GetString(), "Tencent");
}

// Test array parsing
TEST_F(RapidJsonTest, ArrayParsing)
{
  Document doc;
  ASSERT_FALSE(doc.Parse(arrayJson).HasParseError());

  // Check document is an array
  EXPECT_TRUE(doc.IsArray());
  EXPECT_EQ(doc.Size(), 5);

  // Check array values
  for (SizeType i = 0; i < doc.Size(); i++)
  {
    EXPECT_TRUE(doc[i].IsInt());
    EXPECT_EQ(doc[i].GetInt(), i + 1);
  }
}

// Test nested JSON parsing
TEST_F(RapidJsonTest, NestedParsing)
{
  Document doc;
  ASSERT_FALSE(doc.Parse(nestedJson).HasParseError());

  // Check document structure
  EXPECT_TRUE(doc.IsObject());
  EXPECT_TRUE(doc.HasMember("name"));
  EXPECT_TRUE(doc.HasMember("version"));
  EXPECT_TRUE(doc.HasMember("features"));
  EXPECT_TRUE(doc.HasMember("stats"));
  EXPECT_TRUE(doc.HasMember("examples"));

  // Check features array
  EXPECT_TRUE(doc["features"].IsArray());
  EXPECT_EQ(doc["features"].Size(), 4);
  EXPECT_STREQ(doc["features"][0].GetString(), "fast");
  EXPECT_STREQ(doc["features"][1].GetString(), "portable");
  EXPECT_STREQ(doc["features"][2].GetString(), "DOM");
  EXPECT_STREQ(doc["features"][3].GetString(), "SAX");

  // Check stats object
  EXPECT_TRUE(doc["stats"].IsObject());
  EXPECT_TRUE(doc["stats"].HasMember("stars"));
  EXPECT_TRUE(doc["stats"].HasMember("forks"));
  EXPECT_EQ(doc["stats"]["stars"].GetInt(), 12000);
  EXPECT_EQ(doc["stats"]["forks"].GetInt(), 3000);

  // Check examples array of objects
  EXPECT_TRUE(doc["examples"].IsArray());
  EXPECT_EQ(doc["examples"].Size(), 2);
  EXPECT_TRUE(doc["examples"][0].IsObject());
  EXPECT_TRUE(doc["examples"][1].IsObject());
  EXPECT_STREQ(doc["examples"][0]["name"].GetString(), "tutorial");
  EXPECT_STREQ(doc["examples"][1]["name"].GetString(), "pretty");
}

// Test document modification
TEST_F(RapidJsonTest, DocumentModification)
{
  Document doc;
  doc.Parse(simpleJson);

  // Modify existing value
  Value& stars = doc["stars"];
  stars.SetInt(stars.GetInt() + 1);
  EXPECT_EQ(doc["stars"].GetInt(), 11);

  // Add new member
  doc.AddMember("license", "MIT", doc.GetAllocator());
  EXPECT_TRUE(doc.HasMember("license"));
  EXPECT_STREQ(doc["license"].GetString(), "MIT");

  // Remove a member
  EXPECT_TRUE(doc.RemoveMember("author"));
  EXPECT_FALSE(doc.HasMember("author"));

  // Create value and add it
  Value newArray(kArrayType);
  for (int i = 0; i < 3; i++)
  {
    newArray.PushBack(i, doc.GetAllocator());
  }
  doc.AddMember("numbers", newArray, doc.GetAllocator());

  EXPECT_TRUE(doc.HasMember("numbers"));
  EXPECT_TRUE(doc["numbers"].IsArray());
  EXPECT_EQ(doc["numbers"].Size(), 3);
}

// Test JSON stringification
TEST_F(RapidJsonTest, Stringification)
{
  Document doc;
  doc.Parse(simpleJson);

  // Modify and stringify
  doc["stars"].SetInt(42);

  StringBuffer buffer;
  Writer<StringBuffer> writer(buffer);
  doc.Accept(writer);

  std::string result = buffer.GetString();
  EXPECT_NE(result.find("\"stars\":42"), std::string::npos);
  EXPECT_NE(result.find("\"project\":\"rapidjson\""), std::string::npos);
  EXPECT_NE(result.find("\"author\":\"Tencent\""), std::string::npos);

  // Pretty print
  StringBuffer prettyBuffer;
  PrettyWriter<StringBuffer> prettyWriter(prettyBuffer);
  doc.Accept(prettyWriter);

  std::string prettyResult = prettyBuffer.GetString();
  EXPECT_NE(prettyResult.find("\"stars\": 42"), std::string::npos);
  EXPECT_NE(prettyResult.find("\"project\": \"rapidjson\""), std::string::npos);
}

// Test JSON schema validation
TEST_F(RapidJsonTest, SchemaValidation)
{
  // Define a schema
  const char* schemaJson = R"(
        {
            "type": "object",
            "properties": {
                "project": {"type": "string"},
                "stars": {"type": "integer", "minimum": 0},
                "author": {"type": "string"}
            },
            "required": ["project", "stars"]
        }
    )";

  // Parse schema
  Document sd;
  sd.Parse(schemaJson);
  SchemaDocument schema(sd);

  // Valid document
  Document doc1;
  doc1.Parse(simpleJson);
  SchemaValidator validator1(schema);
  EXPECT_TRUE(doc1.Accept(validator1));

  // Invalid document (missing required field)
  Document doc2;
  doc2.Parse("{\"project\":\"rapidjson\"}");
  SchemaValidator validator2(schema);
  EXPECT_FALSE(doc2.Accept(validator2));

  // Invalid document (wrong type)
  Document doc3;
  doc3.Parse("{\"project\":\"rapidjson\",\"stars\":\"many\"}");
  SchemaValidator validator3(schema);
  EXPECT_FALSE(doc3.Accept(validator3));
}

// Test error handling
TEST_F(RapidJsonTest, ErrorHandling)
{
  // Invalid JSON
  const char* invalidJson = "{\"project\":\"rapidjson\",}"; // Extra comma
  Document doc;
  ParseResult result = doc.Parse(invalidJson);

  EXPECT_TRUE(result.IsError());
  EXPECT_NE(result.Offset(), 0);
  EXPECT_NE(result.Code(), kParseErrorNone);

  // Access non-existent member
  Document validDoc;
  validDoc.Parse(simpleJson);
  EXPECT_FALSE(validDoc.HasMember("nonexistent"));

  // Type mismatch
  EXPECT_TRUE(validDoc["project"].IsString());
  EXPECT_FALSE(validDoc["project"].IsNumber());
}

// Test DOM creation from scratch
TEST_F(RapidJsonTest, DomCreation)
{
  Document doc;
  doc.SetObject();
  Document::AllocatorType& allocator = doc.GetAllocator();

  // Add simple values
  doc.AddMember("name", "RapidJSON Test", allocator);
  doc.AddMember("version", 1.0, allocator);
  doc.AddMember("active", true, allocator);

  // Create and add an array
  Value array(kArrayType);
  array.PushBack("one", allocator);
  array.PushBack("two", allocator);
  array.PushBack("three", allocator);
  doc.AddMember("items", array, allocator);

  // Create and add a nested object
  Value object(kObjectType);
  object.AddMember("key1", "value1", allocator);
  object.AddMember("key2", "value2", allocator);
  doc.AddMember("nested", object, allocator);

  // Verify the structure
  EXPECT_TRUE(doc.IsObject());
  EXPECT_TRUE(doc.HasMember("name"));
  EXPECT_TRUE(doc.HasMember("version"));
  EXPECT_TRUE(doc.HasMember("active"));
  EXPECT_TRUE(doc.HasMember("items"));
  EXPECT_TRUE(doc.HasMember("nested"));

  EXPECT_TRUE(doc["items"].IsArray());
  EXPECT_EQ(doc["items"].Size(), 3);

  EXPECT_TRUE(doc["nested"].IsObject());
  EXPECT_TRUE(doc["nested"].HasMember("key1"));
  EXPECT_TRUE(doc["nested"].HasMember("key2"));
}
