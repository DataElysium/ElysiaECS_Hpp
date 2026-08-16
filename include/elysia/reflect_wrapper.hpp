#pragma once

#include <string>
#include <vector>

#include <rfl.hpp>
#include <rfl/json.hpp>
#include <rfl/toml.hpp>
#include <rfl/msgpack.hpp>
#include <rfl/Generic.hpp>
#ifdef ELYSIA_ENABLE_CAPNPROTO
#include <rfl/capnproto.hpp>
#endif

namespace elysia::reflect {
using Generic = rfl::Generic;

template <typename T> inline std::string write_json(const T& obj) { return rfl::json::write(obj); }
template <typename T> inline auto read_json(const std::string& json) { return rfl::json::read<T>(json); }

template <typename T> inline std::string write_toml(const T& obj) { return rfl::toml::write(obj); }
template <typename T> inline auto read_toml(const std::string& toml) { return rfl::toml::read<T>(toml); }

template <typename T> inline std::vector<char> write_msgpack(const T& obj) { return rfl::msgpack::write(obj); }
template <typename T> inline auto read_msgpack(const std::vector<char>& data) { return rfl::msgpack::read<T>(data); }

#ifdef ELYSIA_ENABLE_CAPNPROTO
template <typename T> inline std::vector<char> write_capnp(const T& obj) { return rfl::capnp::write(obj); }
template <typename T> inline auto read_capnp(const std::vector<char>& data) { return rfl::capnp::read<T>(data); }
template <typename T> inline std::string get_capnp_schema() { return rfl::capnp::to_schema<T>().str(); }
#endif
} // namespace elysia::reflect
