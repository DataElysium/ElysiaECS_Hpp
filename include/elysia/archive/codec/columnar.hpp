#pragma once

#include <vector>
#include <string>
#include <map>
#include <optional>
#include <unordered_map>
#include <span>

#include "elysia/archive/model.hpp"
#include "elysia/archive/registry.hpp"
#include "elysia/archive/utils.hpp"
#include "elysia/reflect_wrapper.hpp"
#include "elysia/world.hpp"

namespace elysia::archive {

struct ArchetypeColumnar {
    std::vector<uint64_t> ids;
    std::map<std::string, std::vector<reflect::Generic>> columns;
};

inline Result<void> load_from_columnar(World& world, CommandBuffer& cmd, const SnapshotRegistry& reg, const ResourceEntry& res, const std::unordered_map<uint64_t, Entity>& id_map) {
    std::optional<ArchetypeColumnar> col_data;
    if (res.data) {
        auto col_json = reflect::write_json(*res.data);
        auto res_parse = reflect::read_json<ArchetypeColumnar>(col_json);
        if (res_parse) col_data = *res_parse;
    } else if (res.blob) {
        std::string decoded = base64_decode(*res.blob);
        std::vector<char> bin(decoded.begin(), decoded.end());
        auto res_parse = reflect::read_msgpack<ArchetypeColumnar>(bin);
        if (res_parse) col_data = *res_parse;
    }
    if (!col_data) return Result<void>::err(ErrorCode::InternalError, "Failed to parse columnar data");
    for (size_t i = 0; i < col_data->ids.size(); ++i) {
        if (!id_map.contains(col_data->ids[i])) continue;
        Entity e = id_map.at(col_data->ids[i]);
        for (const auto& [comp_name, entries] : col_data->columns) {
            if (i >= entries.size()) continue;
            for (const auto& [fid, fac] : reg.factories()) {
                if (fac.key == comp_name) {
                    if (fac.generic) {
                        auto res_spawn = fac.generic->from_generic_cmd(cmd, e, entries[i]);
                        if (res_spawn.is_err()) return res_spawn;
                    }
                    break;
                }
            }
        }
    }
    return Result<void>::ok();
}

inline Result<void> load_from_columnar(World& world, CommandBuffer& cmd, const SnapshotRegistry& reg, const ResourceEntry& res, const IDRemapRegistry& id_reg, const IDMapper& mapper) {
    std::optional<ArchetypeColumnar> col_data;
    if (res.data) {
        auto col_json = reflect::write_json(*res.data);
        auto res_parse = reflect::read_json<ArchetypeColumnar>(col_json);
        if (res_parse) col_data = *res_parse;
    } else if (res.blob) {
        std::string decoded = base64_decode(*res.blob);
        std::vector<char> bin(decoded.begin(), decoded.end());
        auto res_parse = reflect::read_msgpack<ArchetypeColumnar>(bin);
        if (res_parse) col_data = *res_parse;
    }
    if (!col_data) return Result<void>::err(ErrorCode::InternalError, "Failed to parse columnar data");
    for (size_t i = 0; i < col_data->ids.size(); ++i) {
        Entity e = mapper.map(col_data->ids[i]);
        for (const auto& [comp_name, entries] : col_data->columns) {
            if (i >= entries.size()) continue;
            for (const auto& [fid, fac] : reg.factories()) {
                if (fac.key == comp_name) {
                    if (fac.generic) {
                        auto res_spawn = fac.generic->from_generic_cmd(cmd, e, entries[i]);
                        if (res_spawn.is_err()) return res_spawn;
                        if (const auto* hook = id_reg.get_hook(fac.type_id)) {
                            void* ptr = cmd.last_payload();
                            if (ptr) (*hook)(ptr, mapper);
                        }
                    }
                    break;
                }
            }
        }
    }
    return Result<void>::ok();
}

inline ArchetypeColumnar export_archetype_to_columnar(Archetype<>& arch, const std::vector<const ComponentFactory*>& active) {
    ArchetypeColumnar col_data;
    col_data.ids.reserve(arch.count());
    for (auto* fac : active) col_data.columns[fac->key].reserve(arch.count());
    for (auto& chunk : arch.table().chunks()) {
        for (size_t k = 0; k < chunk->count(); ++k) {
            col_data.ids.push_back(chunk->entity(k).id());
            for (auto* fac : active) {
                if (fac->generic) {
                    auto col_idx = arch.get_column_index(fac->type_id);
                    void* data = chunk->component(*col_idx, k);
                    col_data.columns[fac->key].push_back(fac->generic->to_generic(data));
                }
            }
        }
    }
    return col_data;
}

} // namespace elysia::archive
