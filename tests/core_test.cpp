#include <gtest/gtest.h>
#include "elysia/elysia.hpp"

using namespace elysia;

TEST(HeaderCore, EntityAndResult) {
    static_assert(sizeof(Entity) == 8);

    auto ok = Result<int>::ok(7);
    ASSERT_TRUE(ok.is_ok());
    EXPECT_EQ(ok.unwrap(), 7);
}

TEST(HeaderCore, EntityIndexLifecycle) {
    EntityIndex index;
    auto e = index.spawn();
    EXPECT_TRUE(index.is_alive(e));
    index.free(e);
    EXPECT_FALSE(index.is_alive(e));
}
