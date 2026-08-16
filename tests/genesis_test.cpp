#include <gtest/gtest.h>
#include <vector>

#include "elysia/elysia.hpp"

TEST(ElysiaGenesis, EntityBitFields) {
    using namespace elysia;

    Entity e(100, 5);
    EXPECT_EQ(e.id(), 100);
    EXPECT_EQ(e.version(), 5);
    EXPECT_TRUE(e.is_valid());

    Entity null_e;
    EXPECT_FALSE(null_e.is_valid());
}

struct Position { float x, y; };
