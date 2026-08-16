set_project("elysia_ecs_header")
set_version("0.1.1")
set_languages("cxxlatest")
set_toolchains("clang")

add_rules("mode.debug", "mode.release")

-- Core Dependencies
add_requires("gtest", "taskflow", "nameof", "raylib")

-- Optional Modules
option("archive")
    set_default(false)
    set_showmenu(true)
    set_description("Enable optional archive/prefab persistence modules")
option_end()

if has_config("archive") then
    add_requires("reflect-cpp 0.24.0", {configs = {msgpack = true, flatbuffers = true, toml = true}})
end

target("ElysiaHeader")
    set_kind("headeronly")
    set_languages("cxxlatest")
    add_includedirs("include", {public = true})
    add_includedirs("thirdparty/ForkUnion/include", {public = true})
    add_packages("nameof", "taskflow", {public = true})
target_end()

target("ElysiaHeaderSmoke")
    set_kind("binary")
    set_languages("cxxlatest")
    add_deps("ElysiaHeader")
    add_files("tests/header_smoke.cpp")
target_end()

target("ElysiaHeaderTests")
    set_kind("binary")
    set_languages("cxxlatest")
    add_deps("ElysiaHeader")
    add_packages("gtest")
    set_pcxxheader("include/elysia/elysia.hpp")
    add_files("tests/main.cpp", "tests/*_test.cpp")
    add_defines("GTEST_HAS_PTHREAD=0")
target_end()

if has_config("archive") then
    target("ElysiaArchiveHeader")
        set_kind("headeronly")
        set_languages("cxxlatest")
        add_deps("ElysiaHeader")
        add_packages("reflect-cpp", {public = true})
    target_end()

    target("ElysiaArchiveHeaderTests")
        set_kind("binary")
        set_languages("cxxlatest")
        add_deps("ElysiaArchiveHeader")
        add_packages("gtest")
        set_pcxxheader("include/elysia/archive/archive.hpp")
        add_files("tests/main.cpp", "tests/archive/*_test.cpp")
        add_defines("GTEST_HAS_PTHREAD=0")
    target_end()
end

target("ElysiaHeaderHelloWorld")
    set_kind("binary")
    set_languages("cxxlatest")
    add_deps("ElysiaHeader")
    add_files("examples/01_hello_world.cpp")
target_end()

target("ElysiaHeaderQueryExample")
    set_kind("binary")
    set_languages("cxxlatest")
    add_deps("ElysiaHeader")
    add_files("examples/header_query_example.cpp")
target_end()

target("ElysiaHeaderBoids")
    set_kind("binary")
    set_languages("cxxlatest")
    add_deps("ElysiaHeader")
    add_packages("raylib")
    add_files("examples/boids_app.cpp")
target_end()

target("ElysiaHeaderSpacewar")
    set_kind("binary")
    set_languages("cxxlatest")
    add_deps("ElysiaHeader")
    add_packages("raylib")
    add_files("examples/spacewar_app.cpp")
target_end()

target("ElysiaHeaderPublicPluginExample")
    set_kind("binary")
    set_languages("cxxlatest")
    add_deps("ElysiaHeader")
    add_files("examples/public_plugin_example.cpp")
target_end()
