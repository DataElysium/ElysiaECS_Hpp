#include <gtest/gtest.h>
#include <string>
#include <vector>
#include "elysia/elysia.hpp"

using namespace elysia;
namespace complex_query{
struct TagA { int val; };
struct TagB { int val; };
struct TagC { int val; };
}
using namespace complex_query;

TEST(ElysiaComplexQuery, MultiArchetypeDataIntegrity) {
    World world;
    
    for(int i=0; i<100; ++i) {
        world.spawn().add(TagA{i}).add(TagB{i * 2});
    }
    for(int i=100; i<200; ++i) {
        world.spawn().add(TagB{i * 2}).add(TagC{i * 3});
    }
    for(int i=200; i<300; ++i) {
        world.spawn().add(TagA{i}).add(TagB{i * 2}).add(TagC{i * 3});
    }
    
    {
        Query<TagA, TagB> q;
        world.update_query(q);
        int count = 0;
        q.each([&](TagA& a, TagB& b) {
            EXPECT_EQ(a.val * 2, b.val);
            count++;
        });
        EXPECT_EQ(count, 200);
    }
    
    {
        Query<TagB, TagC> q;
        world.update_query(q);
        int count = 0;
        q.each([&](TagB& b, TagC& c) {
            EXPECT_EQ(b.val * 3, c.val * 2);
            count++;
        });
        EXPECT_EQ(count, 200);
    }
}

TEST(ElysiaComplexQuery, CompileTimeUniquenessAssert) {
    // This test is mostly for manual verification of static_assert.
    // Uncommenting the next line should cause a compile error.
    // Query<TagA, TagA> q; 
}

TEST(ElysiaComplexQuery, StructuralMigrationDuringQuery) {
    World world;
    std::vector<Entity> entities;
    for(int i=0; i<100; ++i) {
        entities.push_back(world.spawn().add(TagA{i}).entity);
    }
    
    Query<TagA> q;
    world.update_query(q);
    
    for(int i=0; i<50; ++i) {
        world.entity(entities[i]).add(TagB{i});
    }
    
    world.update_query(q);
    
    int count = 0;
    q.each([&](TagA& a) {
        count++;
    });
    EXPECT_EQ(count, 100);
}
