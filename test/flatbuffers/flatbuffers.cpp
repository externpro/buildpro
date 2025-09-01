#include <gtest/gtest.h>

#include "schema_generated.h"

using namespace MyGame::Sample;

class FlatBuffersTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    builder = std::make_unique<flatbuffers::FlatBufferBuilder>();
  }

  void TearDown() override { builder.reset(); }

  std::unique_ptr<flatbuffers::FlatBufferBuilder> builder;
};

TEST_F(FlatBuffersTest, TestMonsterCreation)
{
  // Create a name for our monster
  auto name = builder->CreateString("MyMonster");

  // Create an inventory vector
  unsigned char inv_data[] = {0, 1, 2, 3, 4};
  auto inventory = builder->CreateVector(inv_data, 5);

  // Create a position for the monster using the builder
  auto pos = CreateVec3(*builder, 1.0f, 2.0f, 3.0f);

  // Create the monster
  auto monster = CreateMonster(*builder,
                               pos, // position offset
                               300, // mana
                               200, // hp
                               name,
                               inventory,
                               Color_Green);

  // Finish the buffer
  builder->Finish(monster);

  // Verify the buffer is valid
  auto verifier =
    flatbuffers::Verifier(builder->GetBufferPointer(), builder->GetSize());
  ASSERT_TRUE(VerifyMonsterBuffer(verifier));

  // Get the monster from the buffer
  auto monster_ptr = GetMonster(builder->GetBufferPointer());

  // Verify the data
  ASSERT_NE(monster_ptr, nullptr);
  ASSERT_EQ(monster_ptr->hp(), 200);
  ASSERT_EQ(monster_ptr->mana(), 300);
  ASSERT_STREQ(monster_ptr->name()->c_str(), "MyMonster");
  ASSERT_EQ(monster_ptr->color(), Color_Green);

  auto monster_pos = monster_ptr->pos();
  ASSERT_NE(monster_pos, nullptr);
  ASSERT_FLOAT_EQ(monster_pos->x(), 1.0f);
  ASSERT_FLOAT_EQ(monster_pos->y(), 2.0f);
  ASSERT_FLOAT_EQ(monster_pos->z(), 3.0f);

  auto monster_inv = monster_ptr->inventory();
  ASSERT_NE(monster_inv, nullptr);
  ASSERT_EQ(monster_inv->size(), 5);
  for (int i = 0; i < 5; i++)
  {
    ASSERT_EQ(monster_inv->Get(i), i);
  }
}
