#include <string>

#include <gtest/gtest.h>

#include "schema.pb.h"

using namespace mygame::sample;

class ProtobufTest : public ::testing::Test
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

TEST_F(ProtobufTest, TestMessageCreation)
{
  // Create a monster message
  Monster monster;
  monster.set_name("MyMonster");
  monster.set_hp(200);
  monster.set_mana(300);
  monster.set_color(Color::GREEN);

  // Set position
  Vec3* pos = new Vec3();
  pos->set_x(1.0f);
  pos->set_y(2.0f);
  pos->set_z(3.0f);
  monster.set_allocated_pos(pos);

  // Set inventory
  unsigned char inv_data[] = {0, 1, 2, 3, 4};
  monster.set_inventory(inv_data, sizeof(inv_data));

  // Verify the monster
  EXPECT_EQ(monster.name(), "MyMonster");
  EXPECT_EQ(monster.hp(), 200);
  EXPECT_EQ(monster.mana(), 300);
  EXPECT_EQ(monster.color(), Color::GREEN);
  EXPECT_TRUE(monster.has_pos());
  EXPECT_EQ(monster.pos().x(), 1.0f);
  EXPECT_EQ(monster.pos().y(), 2.0f);
  EXPECT_EQ(monster.pos().z(), 3.0f);
  EXPECT_EQ(monster.inventory().size(), 5);

  // Test serialization
  std::string serialized;
  ASSERT_TRUE(monster.SerializeToString(&serialized));

  // Test deserialization
  Monster parsed_monster;
  ASSERT_TRUE(parsed_monster.ParseFromString(serialized));

  // Compare the messages manually instead of using MessageDifferencer
  EXPECT_EQ(monster.name(), parsed_monster.name());
  EXPECT_EQ(monster.hp(), parsed_monster.hp());
  EXPECT_EQ(monster.mana(), parsed_monster.mana());
  EXPECT_EQ(monster.color(), parsed_monster.color());
  EXPECT_EQ(monster.has_pos(), parsed_monster.has_pos());

  if (monster.has_pos() && parsed_monster.has_pos())
  {
    EXPECT_EQ(monster.pos().x(), parsed_monster.pos().x());
    EXPECT_EQ(monster.pos().y(), parsed_monster.pos().y());
    EXPECT_EQ(monster.pos().z(), parsed_monster.pos().z());
  }

  EXPECT_EQ(monster.inventory(), parsed_monster.inventory());
}

TEST_F(ProtobufTest, TestDefaultValues)
{
  Monster monster;

  // In proto3, primitive fields have default values (0 for numbers, empty for
  // strings/bytes) and message fields are not set by default
  EXPECT_EQ(monster.hp(), 0);
  EXPECT_EQ(monster.mana(), 0);
  EXPECT_EQ(
    monster.color(), Color::RED); // First enum value is default in proto3

  // Position should not be set by default
  EXPECT_FALSE(monster.has_pos());
}

TEST_F(ProtobufTest, TestCopyAndAssignment)
{
  // Create a monster
  Monster monster1;
  monster1.set_name("Monster1");
  monster1.set_hp(100);

  // Test copy constructor
  Monster monster2(monster1);
  EXPECT_EQ(monster2.name(), "Monster1");
  EXPECT_EQ(monster2.hp(), 100);

  // Test assignment operator
  Monster monster3;
  monster3 = monster1;
  EXPECT_EQ(monster3.name(), "Monster1");
  EXPECT_EQ(monster3.hp(), 100);

  // Modify the original and ensure copies are independent
  monster1.set_hp(200);
  EXPECT_EQ(monster1.hp(), 200);
  EXPECT_EQ(monster2.hp(), 100); // Should still be 100
  EXPECT_EQ(monster3.hp(), 100); // Should still be 100
}

TEST_F(ProtobufTest, TestNestedMessages)
{
  // Create a monster with a nested position
  Monster monster;
  monster.set_name("NestedTest");

  // Set position using mutable_pos()
  Vec3* pos = monster.mutable_pos();
  pos->set_x(10.5f);
  pos->set_y(20.5f);
  pos->set_z(30.5f);

  // Verify the nested message
  EXPECT_TRUE(monster.has_pos());
  EXPECT_EQ(monster.pos().x(), 10.5f);
  EXPECT_EQ(monster.pos().y(), 20.5f);
  EXPECT_EQ(monster.pos().z(), 30.5f);

  // Clear the position
  monster.clear_pos();
  EXPECT_FALSE(monster.has_pos());
}

TEST_F(ProtobufTest, TestSerialization)
{
  // Create a test monster
  Monster monster;
  monster.set_name("SerializationTest");
  monster.set_hp(150);
  monster.set_mana(200);

  // Serialize to string
  std::string serialized;
  ASSERT_TRUE(monster.SerializeToString(&serialized));

  // Parse back from string
  Monster parsed_monster;
  ASSERT_TRUE(parsed_monster.ParseFromString(serialized));

  // Verify the parsed monster
  EXPECT_EQ(parsed_monster.name(), "SerializationTest");
  EXPECT_EQ(parsed_monster.hp(), 150);
  EXPECT_EQ(parsed_monster.mana(), 200);
}
