#pragma once

#include <sys/epoll.h>
#include <cstddef>

class EpollEvent {
    struct epoll_event ev_;
public:
    EpollEvent(uint32_t events = 0, void* userData = NULL);

    void setEvents(uint32_t events);
    uint32_t getEvents() const;

    void setUserData(void* ptr);
    void* getUserData() const;

    struct epoll_event* raw();
    const struct epoll_event* raw() const;
};
