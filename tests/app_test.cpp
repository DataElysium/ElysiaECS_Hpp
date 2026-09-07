#include <gtest/gtest.h>
#include <vector>
#include <thread>

#include "elysia/elysia.hpp"

namespace elysia_test::app_test {

using namespace elysia;

struct TagA {};
struct TagB {};

TEST(ElysiaAppHeader, ObserverOnAdd) {
    App app;
    std::vector<Entity> observed_entities;

    app.observer<OnAdd, TagA>().run([&](Entity e) {
        observed_entities.push_back(e);
    });

    app.system("Spawner")
        .run([](World* w) {
            auto& cmd = w->commands().buffer_a();
            Entity e = w->spawn().entity;
            cmd.insert(e, TagA{});
            w->submit(cmd);
        })
        .build();

    app.update();
    EXPECT_EQ(observed_entities.size(), 1u);
}

TEST(ElysiaAppHeader, ObserverFusionSpawn) {
    App app;
    int call_count = 0;

    app.observer<OnAdd, TagA>().run([&](Entity) { call_count++; });

    app.system("FusedSpawner")
        .run([](World* w) {
            auto& cmd = w->commands().buffer_a();
            Entity e = w->spawn().entity;
            cmd.insert(e, TagA{});
            cmd.insert(e, TagB{});
            w->submit(cmd);
        })
        .build();

    app.update();
    EXPECT_EQ(call_count, 1);
}

TEST(ElysiaAppHeader, MultipleObservers) {
    App app;
    int count_a = 0;
    int count_b = 0;

    app.observer<OnAdd, TagA>().run([&](Entity) { count_a++; });
    app.observer<OnAdd, TagB>().run([&](Entity) { count_b++; });

    app.system("DualSpawner")
        .run([](World* w) {
            auto& cmd = w->commands().buffer_a();
            Entity e1 = w->spawn().entity;
            cmd.insert(e1, TagA{});

            Entity e2 = w->spawn().entity;
            cmd.insert(e2, TagB{});
            w->submit(cmd);
        })
        .build();

    app.update();

    EXPECT_EQ(count_a, 1);
    EXPECT_EQ(count_b, 1);
}

TEST(ElysiaApp, KeepsRuntimeWhenChangingExecutors) {
    App app;
    int observed = 0;
    std::thread::id last_thread;
    app.system("Stateful").run([&, calls = 0](World*) mutable {
        observed = ++calls;
        last_thread = std::this_thread::get_id();
    }).build();
    app.init_serial();
    app.update();
    EXPECT_EQ(observed, 1);
    app.init_parallel();
    app.update();
    EXPECT_EQ(observed, 2);
    app.init_serial();
    app.update();
    EXPECT_EQ(observed, 3);
    EXPECT_EQ(last_thread, std::this_thread::get_id());
}

} // namespace elysia_test::app_test
