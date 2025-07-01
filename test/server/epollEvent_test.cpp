#include <gtest/gtest.h>
#include "server/EpollEvent.hpp"

namespace {

TEST(EpollEventTest, ConstructAndRawAccess) {
    int dummy = 42;
    EpollEvent ev(EPOLLIN | EPOLLOUT, &dummy);

    EXPECT_TRUE(ev.hasEvents(EPOLLIN));
    EXPECT_TRUE(ev.hasEvents(EPOLLOUT));
    EXPECT_EQ(ev.getUserData(), &dummy);

    const epoll_event_t* rawEv = ev.raw();
    EXPECT_EQ(rawEv->events, EPOLLIN | EPOLLOUT);
    EXPECT_EQ(rawEv->data.ptr, &dummy);
}

TEST(EpollEventTest, SetAndGetEvents) {
    EpollEvent ev(0, nullptr);
    ev.setEvents(EPOLLERR | EPOLLHUP);
    EXPECT_TRUE(ev.hasEvents(EPOLLERR));
    EXPECT_TRUE(ev.hasEvents(EPOLLHUP));

    ev.removeEvents(EPOLLERR);
    EXPECT_FALSE(ev.hasEvents(EPOLLERR));
    EXPECT_TRUE(ev.hasEvents(EPOLLHUP));

    ev.addEvents(EPOLLIN);
    EXPECT_TRUE(ev.hasEvents(EPOLLIN));
    EXPECT_TRUE(ev.hasEvents(EPOLLHUP));
}

TEST(EpollEventTest, SetAndGetUserData) {
    int data1 = 1;
    int data2 = 2;
    EpollEvent ev(EPOLLIN, &data1);

    EXPECT_EQ(ev.getUserData(), &data1);
    ev.setUserData(&data2);
    EXPECT_EQ(ev.getUserData(), &data2);
}

TEST(EpollEventTest, CopyConstructorFromRaw) {
    int dummy = 99;
    epoll_event_t rawEv;
    rawEv.events = EPOLLIN | EPOLLET;
    rawEv.data.ptr = &dummy;

    EpollEvent ev(rawEv);
    EXPECT_TRUE(ev.hasEvents(EPOLLIN));
    EXPECT_TRUE(ev.hasEvents(EPOLLET));
    EXPECT_EQ(ev.getUserData(), &dummy);
}

} // namespace
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
