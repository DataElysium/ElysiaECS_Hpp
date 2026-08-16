#pragma once
//module;
#include <functional>
#include <cstdint>

//export module elysia.schedule;

#include "elysia/world.hpp"
#include "components_v2.hpp"

#include "graph_types.hpp"
#include "dag_builder_v2.hpp"
#include "traits_v2.hpp"
#include "scheduler.hpp"

#include "scheduler_api_v2.hpp"


// 🌸 Import partitions first
// export import elysia.schedule.components;
// export import :graph_types;   // GraphNode visible to consumers of elysia.schedule
// export import :traits;
// export import :dag_builder;
// export import :base;
// export import :executor;



