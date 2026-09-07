#pragma once
// Shared test component types for header-only tests.
// Share a component only when its definition is identical across translation units.
// Unrelated test types use per-file namespaces in both header and module builds.

namespace elysia::test {

struct Pos { float x = 0, y = 0; };
struct Vel { float dx = 0, dy = 0; };
struct Player {};
struct Dead {};
struct TagA {};
struct TagB {};
struct TagC {};
struct Health { int hp = 100; };
struct Damage { int value = 10; };
struct Renderable {};

} // namespace elysia::test
