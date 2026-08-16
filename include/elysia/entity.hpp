#pragma once

#include <compare>
#include <cstdint>
#include <functional>

#include "elysia/detail/api.hpp"

namespace elysia {

struct Entity {
    uint64_t value;

    static constexpr uint64_t ID_MASK = 0x00000000FFFFFFFF;
    static constexpr uint64_t VERSION_MASK = 0x0000FFFF00000000;
    static constexpr uint64_t TAG_MASK = 0xFFFF000000000000;

    static constexpr uint32_t ID_BITS = 32;
    static constexpr uint32_t VERSION_BITS = 16;
    static constexpr uint32_t TAG_BITS = 16;

    static constexpr uint64_t NULL_VALUE = 0;

    constexpr Entity() : value(NULL_VALUE) {}
    constexpr explicit Entity(uint64_t v) : value(v) {}
    constexpr Entity(uint32_t id, uint16_t ver, uint16_t tag = 0) {
        value = (static_cast<uint64_t>(tag) << (ID_BITS + VERSION_BITS)) |
                (static_cast<uint64_t>(ver) << ID_BITS) |
                static_cast<uint64_t>(id);
    }

    [[nodiscard]] constexpr uint32_t id() const {
        return static_cast<uint32_t>(value & ID_MASK);
    }

    [[nodiscard]] constexpr uint16_t version() const {
        return static_cast<uint16_t>((value & VERSION_MASK) >> ID_BITS);
    }

    [[nodiscard]] constexpr uint16_t tag() const {
        return static_cast<uint16_t>((value & TAG_MASK) >> (ID_BITS + VERSION_BITS));
    }

    [[nodiscard]] constexpr bool is_valid() const { return value != NULL_VALUE; }
    [[nodiscard]] constexpr explicit operator bool() const { return is_valid(); }

    bool operator==(const Entity& other) const = default;
    auto operator<=>(const Entity&) const = delete;

    bool operator<(const Entity& other) const { return value < other.value; }
};

static_assert(sizeof(Entity) == 8, "Entity must be 64-bit");
static_assert(alignof(Entity) == 8, "Entity must be 64-bit aligned");

using entity_t = Entity;

} // namespace elysia

namespace std {
    template<> struct hash<elysia::Entity> {
        std::size_t operator()(const elysia::Entity& e) const noexcept {
            return static_cast<std::size_t>(e.value);
        }
    };
}

