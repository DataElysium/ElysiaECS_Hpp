#include "public_plugin.hpp"

#include <iostream>

#include "elysia/elysia.hpp"

namespace header_plugin_demo {
namespace {
struct StepSystem {
    void operator()(Position& p, const Velocity& v) const {
        p.x += v.x;
        p.y += v.y;
    }
};

struct PrintSystem {
    void operator()(elysia::Entity e, const Position& p) const {
        std::cout << "entity " << e.id() << " pos=(" << p.x << ", " << p.y << ")\n";
    }
};
} // namespace

void DemoPlugin::build(elysia::App& app) {
    app.system("step").run(StepSystem{}).build();
    app.system("print").after("step").run(PrintSystem{}).build();
}

void spawn_demo_entities(elysia::World& world) {
    world.spawn().add(Position{0.0f, 1.0f}).add(Velocity{1.0f, 0.5f});
    world.spawn().add(Position{10.0f, 2.0f}).add(Velocity{-2.0f, 1.0f});
}

} // namespace header_plugin_demo

int main() {
    elysia::App app;
    app.add_plugin(header_plugin_demo::DemoPlugin{});
    header_plugin_demo::spawn_demo_entities(app.world());

    app.update();
    app.update();
    return 0;
}
