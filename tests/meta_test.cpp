#include <gtest/gtest.h>
#include <string>
#include <vector>
#include "elysia/elysia.hpp"

using namespace elysia;

// --- Test Components ---

struct Lenin {
    std::string quote = "State and Revolution";
};

TEST(ElysiaLifecycle, PhysicalInit) {
    World world;
    auto e = world.spawn();
    e.add(Lenin{}); 
    
    auto* l = e.get<Lenin>();
    ASSERT_NE(l, nullptr);
    EXPECT_EQ(l->quote, "State and Revolution");
}

struct Destructible {
    int* counter = nullptr;
    
    Destructible(int* c) : counter(c) {}
    
    ~Destructible() {
        if(counter) (*counter)++;
    }
    
    Destructible(Destructible&& other) noexcept : counter(other.counter) {
        other.counter = nullptr;
    }
    
    Destructible& operator=(Destructible&& other) noexcept {
        if (this != &other) {
            if(counter) (*counter)++; // Clean up current
            counter = other.counter;
            other.counter = nullptr;
        }
        return *this;
    }
};

TEST(ElysiaLifecycle, PhysicalTerm) {
    World world;
    int death_count = 0;
    {
        auto e = world.spawn();
        e.add(Destructible{&death_count});
        EXPECT_EQ(death_count, 0);
        e.despawn();
    }
    EXPECT_EQ(death_count, 1);
}
