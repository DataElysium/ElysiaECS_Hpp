#include <gtest/gtest.h>
#include <string>

#include "elysia/elysia.hpp"

using namespace elysia;

struct Counter { int value = 0; };

TEST(ElysiaSchedulerHeader, MetaWorldExecution) {
    World game_world;
    auto e = game_world.spawn().add(Counter{0}).entity;

    Scheduler scheduler;
    scheduler.add_system("Increment", [](World* w) {
        w->query<Counter>().each([](Counter& c) {
            c.value++;
        });
    });

    auto exec = SerialExecutor::build_from(scheduler);
    exec->run(&game_world);

    auto* c = game_world.get_component<Counter>(e);
    EXPECT_EQ(c->value, 1);

    exec->run(&game_world);
    EXPECT_EQ(c->value, 2);
}

TEST(ElysiaSchedulerHeader, DependencyStructure) {
    World game_world;
    auto e = game_world.spawn().add(Counter{0}).entity;

    Scheduler scheduler;
    scheduler.add_system("SysA", [](World* w) {
        w->query<Counter>().each([](Counter& c) { c.value += 1; });
    });
    scheduler.add_system("SysB", [](World* w) {
        w->query<Counter>().each([](Counter& c) { c.value *= 2; });
    });
    scheduler.add_dependency("SysB", "SysA");

    SerialExecutor::build_from(scheduler)->run(&game_world);

    auto* c = game_world.get_component<Counter>(e);
    EXPECT_EQ(c->value, 2);
}

TEST(ElysiaSchedulerHeader, MultiDependency) {
    World game_world;
    std::string order;

    struct OrderTracker { std::string* str; };
    game_world.resources().add(OrderTracker{&order});

    Scheduler scheduler;
    scheduler.add_system("SysA", [](World* w) {
        if (auto* t = w->get_resource<OrderTracker>()) *(t->str) += "A";
    });
    scheduler.add_system("SysB", [](World* w) {
        if (auto* t = w->get_resource<OrderTracker>()) *(t->str) += "B";
    });
    scheduler.add_system("SysC", [](World* w) {
        if (auto* t = w->get_resource<OrderTracker>()) *(t->str) += "C";
    });

    scheduler.add_dependency("SysB", "SysA");
    scheduler.add_dependency("SysC", "SysA");

    SerialExecutor::build_from(scheduler)->run(&game_world);

    EXPECT_EQ(order.length(), 3u);
    EXPECT_EQ(order[0], 'A');
    EXPECT_TRUE(order == "ABC" || order == "ACB");
}
