#include <gtest/gtest.h>

#include "person.pb.h"

class PersonTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    // Initialize protobuf library
    GOOGLE_PROTOBUF_VERIFY_VERSION;
  }

  void TearDown() override
  {
    // Clean up protobuf library
    google::protobuf::ShutdownProtobufLibrary();
  }
};

TEST_F(PersonTest, TestMessageCreation)
{
  // Create a person message
  test::protobuf_test::Person person;
  person.set_id(123);
  person.set_name("Alice");
  person.set_email("alice@example.com");
  person.set_age(30);

  // Verify the values
  EXPECT_EQ(person.id(), 123);
  EXPECT_EQ(person.name(), "Alice");
  EXPECT_EQ(person.email(), "alice@example.com");
  EXPECT_EQ(person.age(), 30);
}

TEST_F(PersonTest, TestSerialization)
{
  // Create and populate a message
  test::protobuf_test::Person person;
  person.set_id(123);
  person.set_name("Alice");

  // Serialize to string
  std::string serialized;
  ASSERT_TRUE(person.SerializeToString(&serialized));

  // Parse back from string
  test::protobuf_test::Person parsed_person;
  ASSERT_TRUE(parsed_person.ParseFromString(serialized));

  // Verify the parsed message
  EXPECT_EQ(parsed_person.id(), 123);
  EXPECT_EQ(parsed_person.name(), "Alice");
}

TEST_F(PersonTest, TestMessageComparison)
{
  // Create two similar messages
  test::protobuf_test::Person person1;
  person1.set_id(1);
  person1.set_name("Alice");
  person1.set_email("alice@example.com");
  person1.set_age(25);

  test::protobuf_test::Person person2;
  person2.set_id(1);
  person2.set_name("Alice");
  person2.set_email("alice@example.org"); // Different email
  person2.set_age(25);

  // Compare messages manually field by field
  bool same =
    person1.id() == person2.id() && person1.name() == person2.name() &&
    person1.email() == person2.email() && person1.age() == person2.age();

  // Messages should be different
  EXPECT_FALSE(same) << "Expected messages to be different";

  // Verify the specific field that's different
  EXPECT_EQ(person1.id(), person2.id()) << "IDs should be the same";
  EXPECT_EQ(person1.name(), person2.name()) << "Names should be the same";
  EXPECT_NE(person1.email(), person2.email())
    << "Email fields should be different";
  EXPECT_EQ(person1.age(), person2.age()) << "Ages should be the same";
}
