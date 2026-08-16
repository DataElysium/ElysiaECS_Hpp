#include <gtest/gtest.h>
#include <string>

#include "elysia/elysia.hpp"
#include "elysia/archive/archive.hpp"

using namespace elysia;
using namespace elysia::archive;

struct Position { float x, y; };
struct Velocity { float dx, dy; };

TEST(SnapshotRegistryTest, RegisterAndMerge) {
    SnapshotRegistry reg1;
    auto res1 = reg1.register_type<Position>("Position");
    EXPECT_TRUE(res1.is_ok());

    SnapshotRegistry reg2;
    auto res2 = reg2.register_type<Velocity>("Velocity");
    EXPECT_TRUE(res2.is_ok());

    // Merge reg2 into reg1
    auto merge_res = reg1.merge(reg2);
    EXPECT_TRUE(merge_res.is_ok());

    auto& factories = reg1.factories();

    bool has_pos = false;
    bool has_vel = false;

    for (const auto& [id, fac] : factories) {
        if (fac.key == "Position") has_pos = true;
        if (fac.key == "Velocity") has_vel = true;
    }

    EXPECT_TRUE(has_pos);
    EXPECT_TRUE(has_vel);
}

TEST(SnapshotRegistryTest, KeyConflictWithinOneRegistry) {
    SnapshotRegistry reg;
    auto r1 = reg.register_type<Position>("CommonKey");
    EXPECT_TRUE(r1.is_ok());

    auto r2 = reg.register_type<Velocity>("CommonKey");
    EXPECT_TRUE(r2.is_ok());

    EXPECT_TRUE(reg.factories().contains(TypeTraits<Position>::id));
    EXPECT_TRUE(reg.factories().contains(TypeTraits<Velocity>::id));
}

TEST(SnapshotRegistryTest, KeyConflictBetweenRegistries) {
    SnapshotRegistry reg1;
    reg1.register_type<Position>("KeyA");

    SnapshotRegistry reg2;
    reg2.register_type<Velocity>("KeyA");

    auto merge_res = reg1.merge(reg2);
    EXPECT_TRUE(merge_res.is_ok());

    EXPECT_TRUE(reg1.factories().contains(TypeTraits<Position>::id));
    EXPECT_TRUE(reg1.factories().contains(TypeTraits<Velocity>::id));
}

TEST(SnapshotRegistryTest, MergeOverwritePolicy) {
    SnapshotRegistry reg1;
    reg1.register_type<Position>("Pos");

    SnapshotRegistry reg2;
    reg2.register_type<Position>("Pos");

    auto merge_res = reg1.merge(reg2, SnapshotRegistry::MergePolicy::Overwrite);
    EXPECT_TRUE(merge_res.is_ok());
}

