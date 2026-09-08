#include <gtest/gtest.h>
#include <vector>
#include <thread>
#include <stdexcept>

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


TEST(ElysiaAppHeader, DefaultRunnerUpdatesOnce) {
    App app;
    int startup = 0, updates = 0;
    app.add_startup_system("Startup", [&](World*) { ++startup; });
    app.system("Tick").run([&](World*) { ++updates; }).build();
    app.run();
    app.run();
    EXPECT_EQ(startup, 1);
    EXPECT_EQ(updates, 2);
}

TEST(ElysiaAppHeader, PluginRunnerControlsLoopAndKeepsState) {
    struct LoopPlugin {
        void build(App& app) const {
            app.set_runner([calls = 0](App& active) mutable {
                for (int i = 0, count = ++calls; i < count; ++i) active.update();
            });
        }
    };
    App app;
    int updates = 0;
    app.system("Tick").run([&](World*) { ++updates; }).build();
    app.add_plugin(LoopPlugin{});
    app.run();
    app.run();
    EXPECT_EQ(updates, 3);
    app.set_runner({});
    app.run();
    EXPECT_EQ(updates, 4);
}

TEST(ElysiaAppHeader, RunnerCanReturnWithoutInitializingOrUpdating) {
    App app;
    int startup = 0;
    app.add_startup_system("Startup", [&](World*) { ++startup; });
    app.set_runner([&](App& active) { EXPECT_EQ(&active, &app); });
    app.run();
    EXPECT_EQ(startup, 0);
    app.update(); // An external host can still drive the app directly.
    EXPECT_EQ(startup, 1);
}

TEST(ElysiaAppHeader, RunnerGuardsReentryAndPropagatesExceptions) {
    App app;
    app.set_runner([](App& active) {
        EXPECT_THROW(active.run(), std::logic_error);
        EXPECT_THROW(active.set_runner({}), std::logic_error);
        throw std::runtime_error("Loop failed");
    });
    EXPECT_THROW(app.run(), std::runtime_error);
    int called = 0;
    EXPECT_NO_THROW(app.set_runner([&](App&) { ++called; }));
    app.run();
    EXPECT_EQ(called, 1);
}

TEST(ElysiaAppHeader, RunnerPreservesSelectedExecutorAndStartupLifecycle) {
    App app;
    int startup = 0, updates = 0;
    app.add_startup_system("Startup", [&](World*) { ++startup; });
    app.system("Tick").run([&](World*) { ++updates; }).build();
    app.set_runner([](App& active) {
        active.init_parallel();
        active.update();
        active.init_serial();
        active.update();
    });
    app.run();
    EXPECT_EQ(startup, 1);
    EXPECT_EQ(updates, 2);
}

} // namespace elysia_test::app_test
