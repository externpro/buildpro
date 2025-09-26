#include <string>
#include <vector>

#include <gtest/gtest.h>
#include <rapidxml/rapidxml.hpp>

using namespace rapidxml;

class RapidXmlTest : public ::testing::Test
{
protected:
  std::vector<char> xml_buffer;

  bool parseXml(xml_document<>& doc, const std::string& xml_str)
  {
    try
    {
      xml_buffer.assign(xml_str.begin(), xml_str.end());
      xml_buffer.push_back('\0');
      doc.parse<0>(xml_buffer.data());
      return true;
    }
    catch (const std::exception& e)
    {
      std::cerr << "XML Parse Error: " << e.what() << std::endl;
      return false;
    }
  }
};

TEST_F(RapidXmlTest, SimpleParsing)
{
  const std::string xml =
    "<root><person id=\"1\"><name>John</name></person></root>";

  xml_document<> doc;
  ASSERT_TRUE(parseXml(doc, xml)) << "Failed to parse XML";

  xml_node<>* root = doc.first_node();
  ASSERT_NE(root, nullptr) << "Root node not found";

  ASSERT_NE(root->name(), nullptr) << "Root node has no name";
  EXPECT_STREQ(root->name(), "root") << "Unexpected root node name";

  xml_node<>* person = root->first_node("person");
  ASSERT_NE(person, nullptr) << "Person node not found";

  xml_attribute<>* id = person->first_attribute("id");
  ASSERT_NE(id, nullptr) << "ID attribute not found";
  EXPECT_STREQ(id->value(), "1") << "Unexpected ID value";

  xml_node<>* name = person->first_node("name");
  ASSERT_NE(name, nullptr) << "Name node not found";
  EXPECT_STREQ(name->value(), "John") << "Unexpected name value";
}
