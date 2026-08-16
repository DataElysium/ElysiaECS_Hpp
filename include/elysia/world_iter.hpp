#pragma once

#include <cassert>

#include "elysia/world.hpp"
#include "elysia/command.hpp"
#include "elysia/meta.hpp"
#include "elysia/entity.hpp"
#include "elysia/query.hpp"

namespace elysia {

class Iter {
public:
    Iter(World* w, QueryState* q = nullptr, CommandBuffer* cmd = nullptr)
        : world_(w), query_state_(q), cmd_(cmd) {}

    const World& world() const { return *world_; }
    World& world_mut() { return *world_; }

    CommandBuffer& commands() {
        assert(cmd_ && "Iter: CommandBuffer not available for this system!");
        return *cmd_;
    }

    QueryState* query_state() { return query_state_; }

    template <ValidQueryArg... Args>
    auto chunks() {
        assert(query_state_ && "Iter: This system has no pre-baked query!");
        auto* typed_q = static_cast<Query<Args...>*>(query_state_);
        return typed_q->chunks();
    }

    template <ValidQueryArg... Args>
    auto query() {
        return world_->query<Args...>();
    }

private:
    World* world_;
    QueryState* query_state_;
    CommandBuffer* cmd_;
};

} // namespace elysia
