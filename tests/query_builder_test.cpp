#include <gtest/gtest.h>
#include <vector>
#include "elysia/elysia.hpp"

using namespace elysia;
namespace elysia_query_builder_test {
struct Position { float value; };
struct Velocity { float value; };
struct Renderable { };
struct Dead { };
}
using namespace elysia_query_builder_test;

TEST(ElysiaQueryBuilder, DynamicFilterAPI) {
    World world;

    auto e1 = world.spawn()
        .add(Position{10.0f})
        .add(Velocity{5.0f})
        .add(Renderable{})
        .entity;

    world.spawn()
        .add(Position{20.0f})
        .add(Velocity{2.0f});

    world.spawn()
        .add(Position{30.0f})
        .add(Velocity{3.0f})
        .add(Renderable{})
        .add(Dead{});

    auto* p1 = world.get_component<Position>(e1);
    EXPECT_EQ(p1->value, 10.0f);

    int count = 0;
    world.query<Position, const Velocity>()
         .filter<With<Renderable>, Without<Dead>>()
         .each([&](Position& p, const Velocity& v) {
             p.value += v.value;
             count++;
         });

    EXPECT_EQ(count, 1);
    EXPECT_EQ(p1->value, 15.0f);
}
