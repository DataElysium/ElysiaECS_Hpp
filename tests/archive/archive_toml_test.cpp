#include <gtest/gtest.h>
#include <string>
#include <iostream>

#include "elysia/elysia.hpp"
#include "elysia/archive/archive.hpp"

using namespace elysia;
using namespace elysia::archive;

namespace test_toml {
    struct GameConfig {
        float time_scale;
        int difficulty;
    };

    struct Player {
        int hp;
        float x, y;
    };
}

TEST(ElysiaArchive, TomlDumpCheck) {
    World world;
    SnapshotRegistry reg;

    reg.register_resource<test_toml::GameConfig>("GameConfig");
    world.add_resource(test_toml::GameConfig{1.0f, 2});

    reg.register_type<test_toml::Player>("Player");

    world.spawn().add(test_toml::Player{100, 10.0f, 20.0f});
    world.spawn().add(test_toml::Player{50, 5.0f, 5.0f});

    auto snapshot = AuroraArchive::create(world, reg, AuroraArchive::Config{AuroraArchive::Config::Format::Csv});
    snapshot.metadata["Author"] = "Elysia";

    std::string toml_out = reflect::write_toml(snapshot);
    std::cout << "--- World Archive TOML Dump (CSV) ---" << std::endl;
    std::cout << toml_out << std::endl;
    std::cout << "-------------------------------------" << std::endl;

    EXPECT_FALSE(toml_out.empty());
    EXPECT_NE(toml_out.find("Player"), std::string::npos);
    EXPECT_NE(toml_out.find("format = 'csv'"), std::string::npos);
    EXPECT_NE(toml_out.find("Player.hp,Player.x"), std::string::npos);
}

