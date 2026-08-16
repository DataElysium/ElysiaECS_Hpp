#pragma once

#include <cassert>
#include <cstdint>
#include <cstddef>
#include <functional>
#include <memory>
#include <memory_resource>
#include <span>
#include <type_traits>
#include <utility>
#include <vector>

#include "elysia/core.hpp"
#include "elysia/entity.hpp"
#include "elysia/mem.hpp"
#include "elysia/meta.hpp"
#include "elysia/result.hpp"

namespace elysia {

enum class OpCode : uint8_t {
    Spawn = 0, Despawn = 1, Insert = 2, Remove = 3, Call = 4, DespawnId = 5
};

using EraserFn = void(*)(void*);
using RunnerFn = void(*)(void* world, void* payload);

struct ArgMeta {
    const void* metadata;
    void* payload_ptr;
    uint32_t payload_size;
    EraserFn eraser;
    RunnerFn runner;
};

struct CommandHeader {
    static constexpr uint32_t INVALID_META_INDEX = 0xFFFFFFFF;
    OpCode op;
    Entity entity;
    uint32_t meta_index;
};

class CommandBuffer {
public:
    CommandBuffer() : CommandBuffer(nullptr, nullptr) {}

    explicit CommandBuffer(EntityIndex* index, std::shared_ptr<Allocator> upstream = nullptr)
        : index_(index), upstream_(upstream) {
        init();
    }

    void set_index(EntityIndex* index) { index_ = index; }
    EntityIndex* index() const { return index_; }

    void reserve(size_t n) {
        headers_->reserve(n);
        meta_stream_->reserve(n);
    }

    Entity spawn() {
        assert(index_ && "CommandBuffer must have an EntityIndex to use parameterless spawn(). Use set_index() or constructor.");
        uint32_t id = index_->reserve_id();
        uint16_t version = 0;
        if (id < index_->records().size()) version = index_->records()[id].version;
        Entity e(id, version);
        headers_->push_back({OpCode::Spawn, e, CommandHeader::INVALID_META_INDEX});
        return e;
    }

    template <typename Func>
    void spawn_batch(uint32_t count, Func&& callback) {
        assert(index_ && "CommandBuffer must have an EntityIndex to use spawn_batch()");
        auto batch = index_->reserve_batch(count);

        for (uint32_t id : batch.recycled_ids) {
            uint16_t version = index_->records()[id].version;
            Entity e(id, version);
            spawn(e);
            callback(e);
        }

        for (uint32_t i = 0; i < batch.new_count; ++i) {
            uint32_t id = batch.new_start_id + i;
            Entity e(id, 0);
            spawn(e);
            callback(e);
        }
    }

    std::pair<uint32_t, uint32_t> spawn_batch_continuous(uint32_t count) {
        assert(index_ && "CommandBuffer must have an EntityIndex to use spawn_batch_continuous()");
        uint32_t start_id = index_->reserve_continuous(count);
        for (uint32_t i = 0; i < count; ++i) spawn(Entity(start_id + i, 0));
        return {start_id, start_id + count};
    }

    void spawn(Entity e) { headers_->push_back({OpCode::Spawn, e, CommandHeader::INVALID_META_INDEX}); }
    void despawn(Entity e) { headers_->push_back({OpCode::Despawn, e, CommandHeader::INVALID_META_INDEX}); }

    void despawn_id(uint32_t id) {
        headers_->push_back({OpCode::DespawnId, Entity(id, 0), CommandHeader::INVALID_META_INDEX});
    }

    template <typename T>
    void insert(Entity e, T&& component) {
        using RawT = std::remove_cvref_t<T>;
        void* ptr = res_p_->allocate(sizeof(RawT), alignof(RawT));
        new (ptr) RawT(std::forward<T>(component));

        ArgMeta meta {
            .metadata = get_type_info_ptr<RawT>(),
            .payload_ptr = ptr,
            .payload_size = static_cast<uint32_t>(sizeof(RawT)),
            .eraser = nullptr,
            .runner = nullptr
        };

        if constexpr (!std::is_trivially_destructible_v<RawT>) {
            meta.eraser = [](void* p) { static_cast<RawT*>(p)->~RawT(); };
        }

        uint32_t meta_idx = static_cast<uint32_t>(meta_stream_->size());
        meta_stream_->push_back(meta);
        headers_->push_back({OpCode::Insert, e, meta_idx});
    }

    template <typename Func>
    void call(Func&& f) {
        using DecayedFunc = std::decay_t<Func>;
        void* ptr = res_p_->allocate(sizeof(DecayedFunc), alignof(DecayedFunc));
        new (ptr) DecayedFunc(std::forward<Func>(f));

        ArgMeta meta {
            .metadata = nullptr,
            .payload_ptr = ptr,
            .payload_size = static_cast<uint32_t>(sizeof(DecayedFunc)),
            .eraser = [](void* p) { static_cast<DecayedFunc*>(p)->~DecayedFunc(); },
            .runner = [](void* w_ptr, void* p_ptr) { ((*static_cast<DecayedFunc*>(p_ptr))(w_ptr)); }
        };

        uint32_t meta_idx = static_cast<uint32_t>(meta_stream_->size());
        meta_stream_->push_back(meta);
        headers_->push_back({OpCode::Call, Entity(), meta_idx});
    }

    void clear() {
        for (const auto& meta : *meta_stream_) if (meta.eraser) meta.eraser(meta.payload_ptr);
        headers_->clear();
        meta_stream_->clear();
        res_p_->release();
    }

    void reset() { clear(); }

    std::span<const CommandHeader> headers() const { return *headers_; }
    const ArgMeta* get_meta(uint32_t index) const {
        if (index == CommandHeader::INVALID_META_INDEX) return nullptr;
        return &((*meta_stream_)[index]);
    }
    void* get_payload(const ArgMeta& meta) const { return meta.payload_ptr; }

    void* last_payload() {
        if (headers_->empty()) return nullptr;
        const auto& h = headers_->back();
        if (h.meta_index == CommandHeader::INVALID_META_INDEX) return nullptr;
        return (*meta_stream_)[h.meta_index].payload_ptr;
    }

private:
    void init() {
        Allocator* parent = upstream_ ? upstream_.get() : get_default_allocator();
        res_p_ = std::make_unique<std::pmr::monotonic_buffer_resource>(parent);
        headers_ = std::make_unique<std::vector<CommandHeader>>();
        meta_stream_ = std::make_unique<std::vector<ArgMeta>>();
        reserve(128);
    }

    EntityIndex* index_;
    std::shared_ptr<Allocator> upstream_;
    std::unique_ptr<std::pmr::monotonic_buffer_resource> res_p_;
    std::unique_ptr<std::vector<CommandHeader>> headers_;
    std::unique_ptr<std::vector<ArgMeta>> meta_stream_;
};

} // namespace elysia

