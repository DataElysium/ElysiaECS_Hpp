#include <array>
#include <cassert>

#include "elysia/elysia.hpp"

using namespace elysia;

struct HeaderSmokeTag {};
struct HeaderSmokePos { int x = 0; };
struct HeaderSmokeVel { float y = 0.0f; };

int main() {
    static_assert(sizeof(Entity) == 8);

    Result<int> ok = Result<int>::ok(42);
    assert(ok.is_ok());
    assert(ok.unwrap() == 42);

    EntityIndex index;
    auto e = index.spawn();
    index.update(e, nullptr, 0);
    assert(index.is_alive(e));

    ComponentRegistry reg;
    auto* info = get_type_info_ptr<HeaderSmokeTag>();
    auto id = reg.ensure_registered(info);
    assert(id != MetaConfig::INVALID_LOCAL_ID);

    ArchetypeGraph<> graph;
    auto* root = graph.root();
    assert(root != nullptr);

    const auto* pos_info = get_type_info_ptr<HeaderSmokePos>();
    const auto* vel_info = get_type_info_ptr<HeaderSmokeVel>();
    std::array<const TypeInfo*, 2> types{pos_info, vel_info};
    auto* arch = graph.get_or_create(types);
    assert(arch != nullptr);
    assert(arch->has<HeaderSmokePos>());
    assert(arch->has<HeaderSmokeVel>());

    auto* moved = graph.traverse_add(root, pos_info->id);
    assert(moved != nullptr);
    assert(moved != root);

    auto* same = graph.traverse_remove(moved, pos_info->id);
    assert(same == root);

    return 0;
}
