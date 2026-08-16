#pragma once
// Shared test component types for header-only tests.
// In cppm mode each .cpp is a separate module, so duplicate names are fine.
// In header-only mode all headers are merged into one TU via include guard,
// so we must define shared components in exactly ONE place to avoid ODR issues.

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
