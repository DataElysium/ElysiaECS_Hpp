#pragma once

#include <string>
#include <unordered_map>
#include <optional>
#include <cstdint>
#include <functional>
#include <vector>

#include "elysia/world.hpp"
#include "elysia/meta.hpp"
#include "elysia/entity.hpp"
#include "elysia/reflect_wrapper.hpp"
#include "elysia/storage.hpp"
#include "elysia/config.hpp"
#include "elysia/result.hpp"
#include "elysia/archive/model.hpp"

namespace elysia { class World; class CommandBuffer; }

namespace elysia::archive {

struct GenericCodec {
    std::function<reflect::Generic(const void*)> to_generic;
    std::function<Result<void>(World&, Entity, const reflect::Generic&)> from_generic;
    std::function<Result<void>(CommandBuffer&, Entity, const reflect::Generic&)> from_generic_cmd;
};

struct MsgPackCodec {
    std::function<std::vector<char>(const void*)> encode;
    std::function<Result<void>(CommandBuffer&, Entity, const std::vector<char>&)> decode_cmd;
};

struct ComponentFactory {
    uint64_t type_id;
    std::string key;
    size_t size;
    const TypeInfo* info;
    std::optional<GenericCodec> generic;
    std::optional<MsgPackCodec> msgpack;
};

class SnapshotRegistry {
public:
    enum class MergePolicy { KeepExisting, Overwrite };

    Result<void> merge(const SnapshotRegistry& other, MergePolicy policy = MergePolicy::KeepExisting) {
        for (const auto& [id, fac] : other.factories_) {
            bool should_insert = !factories_.contains(id) || policy == MergePolicy::Overwrite;
            if (should_insert) {
                factories_[id] = fac;
                key_to_id_[fac.key] = id;
            } else if (!key_to_id_.contains(fac.key)) {
                key_to_id_[fac.key] = id;
            }
        }
        return Result<void>::ok();
    }

    template <typename T>
    Result<void> register_type(const std::string& key_override = "") {
        ComponentFactory fac;
        fac.type_id = TypeTraits<T>::id;
        fac.key = key_override.empty() ? std::string(TypeTraits<T>::name()) : key_override;
        fac.size = sizeof(T);
        fac.info = get_type_info_ptr<T>();
        enable_generic_codec<T>(fac);
        enable_msgpack_codec<T>(fac);
        factories_[fac.type_id] = std::move(fac);
        key_to_id_[fac.key] = fac.type_id;
        return Result<void>::ok();
    }

    template <typename T, typename T1>
    Result<void> register_type_with_proxy(const std::string& key_override = "") {
        ComponentFactory fac;
        fac.type_id = TypeTraits<T>::id;
        fac.key = key_override.empty() ? std::string(TypeTraits<T>::name()) : key_override;
        fac.size = sizeof(T);
        fac.info = get_type_info_ptr<T>();
        GenericCodec gc;
        gc.to_generic = [](const void* ptr) {
            T1 proxy = T1::from(*static_cast<const T*>(ptr));
            auto json = reflect::write_json(proxy);
            return reflect::read_json<reflect::Generic>(json).value();
        };
        gc.from_generic = [](World& w, Entity e, const reflect::Generic& g) -> Result<void> {
            auto json = reflect::write_json(g);
            auto res = reflect::read_json<T1>(json);
            if (res) { w.entity(e).add(res->into()); return Result<void>::ok(); }
            return Result<void>::err(ErrorCode::InternalError, "Proxy deserialization failed");
        };
        gc.from_generic_cmd = [](CommandBuffer& cmd, Entity e, const reflect::Generic& g) -> Result<void> {
            auto json = reflect::write_json(g);
            auto res = reflect::read_json<T1>(json);
            if (res) { cmd.insert(e, res->into()); return Result<void>::ok(); }
            return Result<void>::err(ErrorCode::InternalError, "Proxy deserialization failed");
        };
        fac.generic = std::move(gc);
        MsgPackCodec mc;
        mc.encode = [](const void* ptr) {
            T1 proxy = T1::from(*static_cast<const T*>(ptr));
            return reflect::write_msgpack(proxy);
        };
        mc.decode_cmd = [](CommandBuffer& cmd, Entity e, const std::vector<char>& data) -> Result<void> {
            auto res = reflect::read_msgpack<T1>(data);
            if (res) { cmd.insert(e, res->into()); return Result<void>::ok(); }
            return Result<void>::err(ErrorCode::InternalError, "Proxy MsgPack decode failed");
        };
        fac.msgpack = std::move(mc);
        factories_[fac.type_id] = std::move(fac);
        key_to_id_[fac.key] = fac.type_id;
        return Result<void>::ok();
    }

    template <typename T>
    void register_resource(const std::string& key_override = "") {
        ComponentFactory fac;
        fac.type_id = TypeTraits<T>::id;
        fac.key = key_override.empty() ? std::string(TypeTraits<T>::name()) : key_override;
        fac.size = sizeof(T);
        fac.info = get_type_info_ptr<T>();
        enable_resource_codec<T>(fac);
        resource_factories_[fac.type_id] = std::move(fac);
    }

    const std::unordered_map<uint64_t, ComponentFactory>& factories() const { return factories_; }
    const std::unordered_map<uint64_t, ComponentFactory>& resource_factories() const { return resource_factories_; }

    SnapshotRegistry& register_type_factory_direct(ComponentFactory fac) {
        key_to_id_[fac.key] = fac.type_id;
        factories_[fac.type_id] = std::move(fac);
        return *this;
    }

private:
    template <typename T>
    void enable_generic_codec(ComponentFactory& fac) {
        GenericCodec c;
        c.to_generic = [](const void* ptr) {
            auto json = reflect::write_json(*static_cast<const T*>(ptr));
            return reflect::read_json<reflect::Generic>(json).value();
        };
        c.from_generic = [](World& w, Entity e, const reflect::Generic& g) -> Result<void> {
            auto json = reflect::write_json(g);
            auto res = reflect::read_json<T>(json);
            if (res) { w.entity(e).add(std::move(*res)); return Result<void>::ok(); }
            return Result<void>::err(ErrorCode::InternalError, "Deserialization failed");
        };
        c.from_generic_cmd = [](CommandBuffer& cmd, Entity e, const reflect::Generic& g) -> Result<void> {
            auto json = reflect::write_json(g);
            auto res = reflect::read_json<T>(json);
            if (res) { cmd.insert(e, std::move(*res)); return Result<void>::ok(); }
            return Result<void>::err(ErrorCode::InternalError, "Deserialization failed");
        };
        fac.generic = std::move(c);
    }

    template <typename T>
    void enable_msgpack_codec(ComponentFactory& fac) {
        MsgPackCodec c;
        c.encode = [](const void* ptr) {
            return reflect::write_msgpack(*static_cast<const T*>(ptr));
        };
        c.decode_cmd = [](CommandBuffer& cmd, Entity e, const std::vector<char>& data) -> Result<void> {
            auto res = reflect::read_msgpack<T>(data);
            if (res) { cmd.insert(e, std::move(*res)); return Result<void>::ok(); }
            return Result<void>::err(ErrorCode::InternalError, "MsgPack decode failed");
        };
        fac.msgpack = std::move(c);
    }

    template <typename T>
    void enable_resource_codec(ComponentFactory& fac) {
        GenericCodec c;
        c.to_generic = [](const void* ptr) {
            auto json = reflect::write_json(*static_cast<const T*>(ptr));
            return reflect::read_json<reflect::Generic>(json).value();
        };
        c.from_generic = [](World& w, Entity, const reflect::Generic& g) -> Result<void> {
            auto json = reflect::write_json(g);
            auto res = reflect::read_json<T>(json);
            if (res) { w.add_resource(std::move(*res)); return Result<void>::ok(); }
            return Result<void>::err(ErrorCode::InternalError, "Resource deserialization failed");
        };
        fac.generic = std::move(c);
    }

    std::unordered_map<uint64_t, ComponentFactory> factories_;
    std::unordered_map<uint64_t, ComponentFactory> resource_factories_;
    std::unordered_map<std::string, uint64_t> key_to_id_;
};

struct IDMapper {
    virtual ~IDMapper() = default;
    virtual Entity map(uint64_t old_id) const = 0;
};

using RemapHook = std::function<void(void* ptr, const IDMapper& mapper)>;

class IDRemapRegistry {
public:
    template <typename T>
    void register_remap_hook(std::function<void(T&, const IDMapper&)> hook) {
        hooks_[TypeTraits<T>::id] = [hook](void* ptr, const IDMapper& m) {
            hook(*static_cast<T*>(ptr), m);
        };
    }

    const RemapHook* get_hook(uint64_t type_id) const {
        if (auto it = hooks_.find(type_id); it != hooks_.end()) return &it->second;
        return nullptr;
    }

private:
    std::unordered_map<uint64_t, RemapHook> hooks_;
};

} // namespace elysia::archive

