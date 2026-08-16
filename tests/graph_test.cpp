#include <gtest/gtest.h>
#include <array>
#include "elysia/elysia.hpp"

using namespace elysia;

struct HeaderGraphPos { int x = 0; };
struct HeaderGraphVel { float y = 0.0f; };

TEST(HeaderGraph, CreateAndTraverse) {
    ArchetypeGraph<> graph;
    auto* root = graph.root();
    ASSERT_NE(root, nullptr);

    const auto* pos = get_type_info_ptr<HeaderGraphPos>();
    const auto* vel = get_type_info_ptr<HeaderGraphVel>();
    std::array<const TypeInfo*, 2> types{pos, vel};

    auto* arch = graph.get_or_create(std::span<const TypeInfo* const>(types.data(), types.size()));
    ASSERT_NE(arch, nullptr);
    EXPECT_TRUE(arch->has<HeaderGraphPos>());
    EXPECT_TRUE(arch->has<HeaderGraphVel>());

    auto* next = graph.traverse_add(root, pos->id);
    ASSERT_NE(next, nullptr);
    EXPECT_NE(next, root);

    auto* back = graph.traverse_remove(next, pos->id);
    EXPECT_EQ(back, root);
}
