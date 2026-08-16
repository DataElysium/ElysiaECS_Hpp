#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "elysia/reflect_wrapper.hpp"

namespace elysia::archive {

struct ResourceEntry {
    std::string format;
    std::string encoding;
    std::optional<reflect::Generic> data;
    std::optional<std::string> blob;
};

struct EntitySegment { uint64_t start; uint32_t count; };
struct ArchetypeBlob {
    std::string name;
    std::string table_str;
    std::vector<std::string> components;
    std::string source_uri;
};

struct WorldArchive {
    std::string version = "1.0";
    std::vector<EntitySegment> entities;
    std::vector<ArchetypeBlob> archetypes;
    std::unordered_map<std::string, ResourceEntry> embed;
    std::unordered_map<std::string, reflect::Generic> resources;
    std::unordered_map<std::string, std::string> metadata;
};

} // namespace elysia::archive
