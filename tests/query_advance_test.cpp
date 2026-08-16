#include <gtest/gtest.h>
#include <vector>
#include "elysia/elysia.hpp"

using namespace elysia;

struct TestPos47 { float x, y; };
struct TestVel47 { float x, y; };
struct Player {};
struct Dead {};
struct TestGlobalConfig47 { float gravity = 9.8f; };

TEST(ElysiaQuery4_7, MixedArguments) {
    World world;
    world.resources().add(TestGlobalConfig47{1.6f});
    
    world.spawn().add(TestPos47{0, 10}).add(TestVel47{0, -1}).add(Player{});
    world.spawn().add(TestPos47{0, 20}).add(TestVel47{0, -1});
    world.spawn().add(TestPos47{0, 30}).add(TestVel47{0, -1}).add(Player{}).add(Dead{});

    Query<TestPos47, With<Player>, Res<TestGlobalConfig47>, Without<Dead>, TestVel47> q;
    world.update_query(q);

    int count = 0;
    q.each(&world, [&](TestPos47& p, TestGlobalConfig47& cfg, TestVel47& v) {
        EXPECT_EQ(p.y, 10);
        EXPECT_FLOAT_EQ(cfg.gravity, 1.6f);
        count++;
    });
    EXPECT_EQ(count, 1);
}

TEST(ElysiaQuery4_7, AutoDeduction) {
    World world;
    world.resources().add(TestGlobalConfig47{5.0f});
    world.spawn().add(TestPos47{1, 1}).add(TestVel47{10, 10});

    auto q = make_query([](TestPos47& p, const TestVel47& v, Res<TestGlobalConfig47> cfg) {
        p.x += v.x * cfg->gravity;
    });
    
    world.update_query(q);
    q.each(&world, [](TestPos47& p, const TestVel47& v, TestGlobalConfig47& cfg) {
        p.x += v.x * cfg.gravity;
    });

    auto* p = world.get_component<TestPos47>(Entity(0, 0));
    EXPECT_FLOAT_EQ(p->x, 1.0f + 10.0f * 5.0f);
}

TEST(ElysiaQuery4_7, ConstCorrectness) {
    World world;
    world.spawn().add(TestPos47{100, 100});

    Query<const TestPos47> q;
    world.update_query(q);

    q.each([](const TestPos47& p) {
        EXPECT_EQ(p.x, 100);
    });
}
