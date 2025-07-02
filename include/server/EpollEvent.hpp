#pragma once

#include <sys/epoll.h>
#include <cstddef>
typedef struct epoll_event epoll_event_t;

class EpollEvent {
private:
    epoll_event_t ev_;
public:
    explicit EpollEvent(const epoll_event_t& ev);
    explicit EpollEvent(uint32_t events = 0, void* userData = NULL);
    void setEvents(uint32_t events);
    uint32_t getEvents() const;
    void setUserData(void* ptr);
    void* getUserData() const;
    void addEvents(uint32_t events);
    void removeEvents(uint32_t events);
    bool hasEvents(uint32_t events) const;
    epoll_event_t* raw();
    const epoll_event_t* raw() const;
};
