#pragma once

#include <algorithm>
#include <iostream>
#include <vector>

#include "elysia/elysia.hpp"

namespace elysia {

struct ChildOf { Entity parent; };
struct Children { std::vector<Entity> ids; };

struct HierarchyPlugin {
    void build(App& app) {
        app.observer<OnAdd, ChildOf>().run([&](Entity e) {
            auto* child_of = app.world().get_component<ChildOf>(e);
            if (!child_of) {
                std::cout << "[Hierarchy] ChildOf missing on entity " << e.id() << std::endl;
                return;
            }

            Entity parent = child_of->parent;
            if (!app.world().index().is_alive(parent)) {
                std::cout << "[Hierarchy] Parent " << parent.id() << " is dead!" << std::endl;
                return;
            }

            auto* children = app.world().get_component<Children>(parent);
            if (!children) {
                app.world().entity(parent).add(Children{});
                children = app.world().get_component<Children>(parent);
            }

            if (children) {
                children->ids.push_back(e);
            } else {
                std::cout << "[Hierarchy] FAILED to get Children component after adding!" << std::endl;
            }
        });

        app.observer<OnRemove, Children>().run([&](Entity e) {
            auto* children = app.world().get_component<Children>(e);
            if (children) {
                for (Entity child : children->ids) {
                    if (app.world().index().is_alive(child)) {
                        app.world().despawn(child);
                    }
                }
            }
        });

        app.observer<OnRemove, ChildOf>().run([&](Entity e) {
            auto* child_of = app.world().get_component<ChildOf>(e);
            if (child_of && app.world().index().is_alive(child_of->parent)) {
                auto* children = app.world().get_component<Children>(child_of->parent);
                if (children) {
                    std::erase(children->ids, e);
                }
            }
        });
    }
};

} // namespace elysia
