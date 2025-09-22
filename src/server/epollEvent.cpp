#include "server/EpollEvent.hpp"

EpollEvent::EpollEvent(uint32_t events, void* userData) {
    ev_.events = events;
    ev_.data.ptr = userData;
}

EpollEvent::EpollEvent(const epoll_event_t& ev): ev_(ev) {
}

uint32_t EpollEvent::getEvents() const {
    return ev_.events;
}

void EpollEvent::setEvents(uint32_t events) {
    ev_.events = events;
}

void EpollEvent::setUserData(void* ptr){
    ev_.data.ptr = ptr;
}

void* EpollEvent::getUserData() const {
    return ev_.data.ptr;
}

void EpollEvent::addEvents(uint32_t events) {
    ev_.events |= events;
}

void EpollEvent::removeEvents(uint32_t events) {
    ev_.events &= ~events;
}

bool EpollEvent::hasEvents(uint32_t events) const {
    return (ev_.events & events) == events;
}

epoll_event_t* EpollEvent::raw() {
    return &ev_;
}

const epoll_event_t* EpollEvent::raw() const {
    return &ev_;
}