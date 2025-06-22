#pragma once

#include <sys/epoll.h>
#include <cstddef>

class EpollEvent {
    struct epoll_event ev_;
public:
    EpollEvent(uint32_t events = 0, void* userData = NULL) {
        ev_.events = events;
        ev_.data.ptr = userData;
    }
    void setEvents(uint32_t events) { ev_.events = events; }
    uint32_t getEvents() const { return ev_.events; }

    void setUserData(void* ptr) { ev_.data.ptr = ptr; }
    void* getUserData() const { return ev_.data.ptr; }

    struct epoll_event* raw() { return &ev_; }
    const struct epoll_event* raw() const { return &ev_; }
};
