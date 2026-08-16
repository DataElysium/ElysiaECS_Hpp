#pragma once

#include <cstddef>
#include <cstring>
#include <span>
#include <string>
#include <vector>
#include <concepts>

#include "elysia/result.hpp"

namespace elysia::archive {

template <typename T>
concept OutputStream = requires(T& t, const void* data, size_t size) {
    { t.write_bytes(data, size) } -> std::same_as<Result<void>>;
};

template <typename T>
concept InputStream = requires(T& t, void* data, size_t size) {
    { t.read_bytes(data, size) } -> std::same_as<Result<void>>;
};

template <typename T>
concept ZeroCopyInputStream = InputStream<T> && requires(T& t, size_t size) {
    { t.view_bytes(size) } -> std::same_as<Result<std::span<const char>>>;
};

struct VectorWriter {
    std::vector<char>& buffer;
    Result<void> write_bytes(const void* data, size_t size) {
        size_t old_size = buffer.size();
        buffer.resize(old_size + size);
        std::memcpy(buffer.data() + old_size, data, size);
        return Result<void>::ok();
    }
};

struct SpanReader {
    std::span<const char> data;
    size_t cursor = 0;

    SpanReader(std::span<const char> s) : data(s) {}
    SpanReader(const std::vector<char>& v) : data(v) {}
    SpanReader(const std::string& s) : data(s) {}

    Result<void> read_bytes(void* dest, size_t size) {
        if (cursor + size > data.size()) return Result<void>::err(ErrorCode::OutOfBounds, "Stream EOF");
        std::memcpy(dest, data.data() + cursor, size);
        cursor += size;
        return Result<void>::ok();
    }

    Result<std::span<const char>> view_bytes(size_t size) {
        if (cursor + size > data.size()) return Result<std::span<const char>>::err(ErrorCode::OutOfBounds, "Stream EOF");
        auto s = data.subspan(cursor, size);
        cursor += size;
        return Result<std::span<const char>>::ok(s);
    }

    bool is_eof() const { return cursor >= data.size(); }
};

} // namespace elysia::archive
