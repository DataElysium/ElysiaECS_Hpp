#include <iostream>

#include "elysia/elysia.hpp"

using namespace elysia;

struct Position { float x, y; };
struct Velocity { float dx, dy; };
struct Mass { float value; };

struct StaticTag {};
struct ActiveTag {};

struct PhysicsConfig {
    float time_step = 0.016f;
    float global_friction = 0.98f;
};

int main() {
    World world;
    world.resources().add(PhysicsConfig{0.016f, 0.98f});

    auto active = world.spawn()
        .add(Position{0.0f, 0.0f})
        .add(Velocity{10.0f, 5.0f})
        .add(Mass{1.0f})
        .add(ActiveTag{})
        .entity;

    world.spawn()
        .add(Position{100.0f, 100.0f})
        .add(Velocity{0.0f, 0.0f})
        .add(StaticTag{});

    auto physics = world.query<Entity, Position, Velocity, Res<PhysicsConfig>>()
        .filter<With<ActiveTag>, Without<StaticTag>>();

    std::cout << "Elysia header-only physics example\n";
    for (int frame = 0; frame < 5; ++frame) {
        physics.each([&](Entity e, Position& p, Velocity& v, Res<PhysicsConfig> cfg) {
            p.x += v.dx * cfg->time_step;
            p.y += v.dy * cfg->time_step;
            v.dx *= cfg->global_friction;
            v.dy *= cfg->global_friction;

            std::cout << "frame " << frame
                      << " entity " << e.id()
                      << " pos=(" << p.x << ", " << p.y << ")"
                      << " vel=(" << v.dx << ", " << v.dy << ")\n";
        });
    }

    auto* p = world.get_component<Position>(active);
    if (!p) return 1;
    std::cout << "final active position: " << p->x << ", " << p->y << "\n";
    return 0;
}
