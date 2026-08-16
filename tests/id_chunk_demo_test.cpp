#include <gtest/gtest.h>
#include <vector>

#include "elysia/elysia.hpp"

using namespace elysia;

struct Mark { int val; };

// Test 1: chunk exactly enough - normal scenario
TEST(IdChunkExhaustion, ChunkExactlyEnough) {
    const int CHUNK = 4;
    World world;

    std::vector<Entity> chunk;
    for (int i = 0; i < CHUNK; ++i)
        chunk.push_back(world.index().spawn());
    size_t cursor = 0;

    CommandBuffer cmd(&world.index());
    for (int i = 0; i < 4; ++i) {
        ASSERT_LT(cursor, chunk.size()) << "chunk exhausted at i=" << i;
        Entity e = chunk[cursor++];
        cmd.spawn(e);
        cmd.insert(e, Mark{i});
    }
    EXPECT_EQ(cursor, CHUNK);

    world.submit(cmd);
    int count = 0;
    world.query<Entity, Mark>().each([&](Entity, Mark) { count++; });
    EXPECT_EQ(count, 4);
}

// Test 2: chunk not enough, cursor >= size -> stop
TEST(IdChunkExhaustion, ChunkExhausted_OverflowDetected) {
    const int CHUNK = 3;
    const int WANT_SPAWN = 10;

    World world;
    std::vector<Entity> chunk;
    for (int i = 0; i < CHUNK; ++i)
        chunk.push_back(world.index().spawn());
    size_t cursor = 0;

    CommandBuffer cmd(&world.index());
    int spawned = 0;

    for (int i = 0; i < WANT_SPAWN; ++i) {
        if (cursor >= chunk.size()) break;
        Entity e = chunk[cursor++];
        cmd.spawn(e);
        cmd.insert(e, Mark{i});
        spawned++;
    }

    EXPECT_EQ(spawned, CHUNK) << "must stop at chunk boundary";
    EXPECT_GT(WANT_SPAWN, spawned) << "remaining spawns would need defer or retry";

    world.submit(cmd);
    int count = 0;
    world.query<Entity, Mark>().each([&](Entity, Mark) { count++; });
    EXPECT_EQ(count, CHUNK);
}
