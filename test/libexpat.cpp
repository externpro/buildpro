#include <cstring>
#include <string>
#include <vector>

#include <expat.h>
#include <gtest/gtest.h>

// Structure to hold parsing context
struct ParseContext
{
  std::vector<std::string> elementStack;
  std::string characterData;
  std::string lastAttributeName;
  std::string lastAttributeValue;
  std::string version;
  std::string encoding;
  bool startElementCalled = false;
  bool endElementCalled = false;
  bool characterDataHandled = false;

  // Reset the context for a new parse
  void reset()
  {
    elementStack.clear();
    characterData.clear();
    lastAttributeName.clear();
    lastAttributeValue.clear();
    version.clear();
    encoding.clear();
    startElementCalled = false;
    endElementCalled = false;
    characterDataHandled = false;
  }
};

// Expat callback for element start
static void XMLCALL StartElementHandler(void* userData,
                                        const XML_Char* name,
                                        const XML_Char** atts)
{
  ParseContext* context = static_cast<ParseContext*>(userData);
  context->elementStack.push_back(name);
  context->startElementCalled = true;

  // Process attributes if any
  if (atts)
  {
    for (int i = 0; atts[i]; i += 2)
    {
      context->lastAttributeName = atts[i];
      context->lastAttributeValue = atts[i + 1];
    }
  }
}

// Expat callback for element end
static void XMLCALL EndElementHandler(void* userData, const XML_Char* name)
{
  ParseContext* context = static_cast<ParseContext*>(userData);
  if (!context->elementStack.empty() && context->elementStack.back() == name)
  {
    context->elementStack.pop_back();
  }
  context->endElementCalled = true;
}

// Expat callback for character data
static void XMLCALL CharacterDataHandler(void* userData,
                                         const XML_Char* s,
                                         int len)
{
  ParseContext* context = static_cast<ParseContext*>(userData);
  context->characterData.append(s, len);
  context->characterDataHandled = true;
}

class ExpatTest : public ::testing::Test
{
protected:
  XML_Parser parser;
  ParseContext context;

  void SetUp() override
  {
    // Create a new parser with UTF-8 encoding
    parser = XML_ParserCreate("UTF-8");
    ASSERT_NE(parser, nullptr) << "Failed to create XML parser";

    // Reset the context
    context.reset();

    // Set up the parser with our callbacks
    XML_SetUserData(parser, &context);
    XML_SetElementHandler(parser, StartElementHandler, EndElementHandler);
    XML_SetCharacterDataHandler(parser, CharacterDataHandler);

    // Set up XML declaration handler
    XML_SetXmlDeclHandler(
      parser,
      [](void* userData, const XML_Char* version, const XML_Char* encoding, int)
      {
        ParseContext* ctx = static_cast<ParseContext*>(userData);
        if (version)
          ctx->version = version;
        if (encoding)
          ctx->encoding = encoding;
      });
  }

  void TearDown() override
  {
    if (parser)
    {
      XML_ParserFree(parser);
      parser = nullptr;
    }
  }

  bool ParseString(const std::string& xml)
  {
    context.reset(); // Reset context using our reset function
    return XML_Parse(
             parser, xml.data(), static_cast<int>(xml.length()), XML_TRUE) ==
           XML_STATUS_OK;
  }

  std::string GetErrorString()
  {
    enum XML_Error code = XML_GetErrorCode(parser);
    return std::string(XML_ErrorString(code)) + " at line " +
           std::to_string(XML_GetCurrentLineNumber(parser)) + ", column " +
           std::to_string(XML_GetCurrentColumnNumber(parser));
  }
};

TEST_F(ExpatTest, BasicParsing)
{
  // Simple XML for testing basic parsing
  const char* xml = "<root><greeting>Hello, World!</greeting><test "
                    "attr=\"value\">Content</test></root>";

  // Parse the XML
  int status = XML_Parse(parser, xml, static_cast<int>(strlen(xml)), 1);
  EXPECT_EQ(XML_STATUS_OK, status)
    << "Parsing failed: " << XML_ErrorString(XML_GetErrorCode(parser));

  // Check that our callbacks were called
  EXPECT_TRUE(context.startElementCalled)
    << "StartElement handler was not called";
  EXPECT_TRUE(context.endElementCalled) << "EndElement handler was not called";
  EXPECT_TRUE(context.characterDataHandled)
    << "CharacterData handler was not called";

  // Check element stack is empty (all elements properly closed)
  EXPECT_TRUE(context.elementStack.empty()) << "Element stack is not empty";

  // Check that we captured the character data
  EXPECT_NE(context.characterData.find("Hello, World!"), std::string::npos)
    << "Character data not found in: " << context.characterData;

  // Check attribute handling
  EXPECT_EQ(context.lastAttributeName, "attr") << "Attribute name not captured";
  EXPECT_EQ(context.lastAttributeValue, "value")
    << "Attribute value not captured";
}

TEST_F(ExpatTest, VersionInfo)
{
  // Get Expat version information
  XML_Expat_Version version = XML_ExpatVersionInfo();

  // Print version information
  std::cout << "Expat version: " << XML_ExpatVersion() << std::endl;
  std::cout << "Major: " << version.major << ", "
            << "Minor: " << version.minor << ", "
            << "Micro: " << version.micro << std::endl;

  // Basic version checks
  EXPECT_GE(version.major, 2) << "Unexpected major version";
  EXPECT_GE(version.minor, 0) << "Unexpected minor version";
  EXPECT_GE(version.micro, 0) << "Unexpected micro version";
}

TEST_F(ExpatTest, ErrorHandling)
{
  // Test with malformed XML (missing closing tag)
  const char* bad_xml = "<?xml version=\"1.0\"?>"
                        "<root>"
                        "    <unclosed>"
                        "</root>";

  // Parse the XML
  context = ParseContext(); // Reset context
  XML_Status status =
    XML_Parse(parser, bad_xml, static_cast<int>(strlen(bad_xml)), XML_TRUE);

  // Should fail due to unclosed <unclosed> tag
  EXPECT_NE(status, XML_STATUS_OK)
    << "Expected parsing to fail due to unclosed tag";

  // Check that we got an error code
  enum XML_Error error = XML_GetErrorCode(parser);
  EXPECT_NE(error, XML_ERROR_NONE) << "Expected an error code";

  // Check error message contains something useful
  const char* errorStr = XML_ErrorString(error);
  EXPECT_NE(errorStr, nullptr) << "Expected an error message";
  if (errorStr)
  {
    EXPECT_NE(std::string(errorStr).find("mismatched tag"), std::string::npos)
      << "Expected error about mismatched tags";
  }
}
