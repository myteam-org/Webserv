#include "EpollEvent.hpp"

EpollEvent::EpollEvent(uint32_t events, void* userData) {
	ev_.events = events;
	ev_.data.ptr = userData;
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

struct epoll_event* EpollEvent::raw() {
	return ev_;
}

const struct epoll_event* EpollEvent::raw() const {
	return &ev_;
}