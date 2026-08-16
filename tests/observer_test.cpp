#include <gtest/gtest.h>
#include "elysia/elysia.hpp"

using namespace elysia;

struct HeaderObserverTag {};

TEST(HeaderObserver, RegisterAndNotify) {
    ObserverRegistry reg;
    Entity e(7, 1);
    bool invoked = false;

    reg.on_add<HeaderObserverTag>([&](Entity got) {
        invoked = (got == e);
    });

    EXPECT_TRUE(reg.has_observer(ObserverEvent::OnAdd, TypeTraits<HeaderObserverTag>::id));
    reg.notify(ObserverEvent::OnAdd, TypeTraits<HeaderObserverTag>::id, e);
    EXPECT_TRUE(invoked);
}
