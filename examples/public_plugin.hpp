#pragma once

#include "elysia/fwd.hpp"

namespace header_plugin_demo {

struct Position {
    float x = 0.0f;
    float y = 0.0f;
};

struct Velocity {
    float x = 0.0f;
    float y = 0.0f;
};

struct DemoPlugin {
    void build(elysia::App& app);
};

void spawn_demo_entities(elysia::World& world);

} // namespace header_plugin_demo
