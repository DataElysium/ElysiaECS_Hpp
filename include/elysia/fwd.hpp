#pragma once

#include "elysia/detail/api.hpp"
#include "elysia/entity.hpp"

namespace elysia {

class App;
class CommandBuffer;
class Scheduler;
class World;
class Iter;

struct CommandAccessor;
struct EntityView;
struct ResourceAccessor;
struct WorldBatchAccessor;
struct WorldView;

class QueryState;
class ObserverRegistry;

namespace schedule {
    enum class SpecialSystemKind;
    enum class ThreadingModel;
}

} // namespace elysia
