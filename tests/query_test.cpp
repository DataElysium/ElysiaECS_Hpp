#include <gtest/gtest.h>
#include <array>
#include "elysia/elysia.hpp"

using namespace elysia;

struct HeaderQueryPos { int x = 0; };
struct HeaderQueryVel { float y = 0.0f; };

TEST(HeaderQuery, PrepareAndMatch) {
    ArchetypeGraph<> graph;
    const auto* pos = get_type_info_ptr<HeaderQueryPos>();
    const auto* vel = get_type_info_ptr<HeaderQueryVel>();
    std::array<const TypeInfo*, 2> types{pos, vel};

    auto* arch = graph.get_or_create(std::span<const TypeInfo* const>(types.data(), types.size()));
    ASSERT_NE(arch, nullptr);

    Query<HeaderQueryPos, HeaderQueryVel> q;
    q.prepare(graph.registry());
    q.update_archetype(arch);

    EXPECT_FALSE(q.archetypes().empty());
    EXPECT_EQ(q.archetypes().size(), 1u);
    EXPECT_TRUE(q.empty());
}
