#pragma once

#include <bit>

namespace elysia {

struct DefaultConfig {
    static constexpr int ChunkSize = 2048;

    static_assert(ChunkSize == -1 || (ChunkSize > 0 && (ChunkSize & (ChunkSize - 1)) == 0),
        "Elysia Config Error: ChunkSize must be a power of two or -1 (Monolithic).");
};

} // namespace elysia
