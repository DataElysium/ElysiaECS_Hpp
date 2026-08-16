#include <gtest/gtest.h>
#include "elysia/elysia.hpp"

using namespace elysia;

struct HeaderCommandPos { int x = 0; };

TEST(HeaderCommand, SpawnInsertAndClear) {
    EntityIndex index;
    CommandBuffer cmd(&index);

    auto e = cmd.spawn();
    cmd.insert(e, HeaderCommandPos{42});
    cmd.despawn(e);

    ASSERT_EQ(cmd.headers().size(), 3u);
    EXPECT_EQ(cmd.headers()[0].op, OpCode::Spawn);
    EXPECT_EQ(cmd.headers()[1].op, OpCode::Insert);
    EXPECT_EQ(cmd.headers()[2].op, OpCode::Despawn);

    const auto& insert_header = cmd.headers()[1];
    ASSERT_NE(cmd.get_meta(insert_header.meta_index), nullptr);
    auto* payload = static_cast<HeaderCommandPos*>(cmd.get_payload(*cmd.get_meta(insert_header.meta_index)));
    ASSERT_NE(payload, nullptr);
    EXPECT_EQ(payload->x, 42);

    cmd.clear();
    EXPECT_TRUE(cmd.headers().empty());
}
