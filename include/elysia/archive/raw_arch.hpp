#pragma once

#include <vector>

#include "elysia/archive/codec/raw.hpp"
#include "elysia/archive/stream.hpp"

namespace elysia::archive {

class RawArchive {
public:
    static Result<std::vector<char>> pack(World& world, bool unchecked = false) {
        std::vector<char> buffer; VectorWriter writer{buffer};
        auto res = pack_world_raw(writer, world, unchecked);
        if (res.is_err()) return Result<std::vector<char>>::err(res.error().code, res.error().message);
        return Result<std::vector<char>>::ok(std::move(buffer));
    }

    template <typename... T>
    static Result<void> unpack(World& world, const std::vector<char>& data) {
        (world.graph().registry().ensure_registered(get_type_info_ptr<T>()), ...);
        SpanReader reader{data};
        return unpack_world_raw(reader, world);
    }

    template <typename... T>
    static Result<void> unpack(World& world, std::span<const char> data) {
        (world.graph().registry().ensure_registered(get_type_info_ptr<T>()), ...);
        SpanReader reader{data};
        return unpack_world_raw(reader, world);
    }
};

} // namespace elysia::archive
