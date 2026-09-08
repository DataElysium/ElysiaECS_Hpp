#pragma once
#include "elysia/elysia.hpp"
#include "elysia/hierarchy.hpp"

namespace elysia {
struct HierarchyPlugin {
    void build(App& app) { install_hierarchy(app.world()); }
};
}
