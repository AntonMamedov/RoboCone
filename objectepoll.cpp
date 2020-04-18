#include "objectepoll.h"

ObjectEpoll::ObjectEpoll(){
    epollDescriptor = epoll_create(100);
}


ObjectEpoll::~ObjectEpoll(){
    close(epollDescriptor);
}


void ObjectEpoll::AddDescriptor(int fd, uint32_t flags, void* ptr){
    epoll_event event;
    if (ptr != nullptr)
        event.data.ptr = ptr;
    else
        event.data.fd = fd;
    event.events = flags;
    epoll_ctl(epollDescriptor, EPOLL_CTL_ADD, fd, &event);
}


int ObjectEpoll::Wait(epoll_event* events, int maxEvents, int timeOut){
    return epoll_wait(epollDescriptor, events, maxEvents, timeOut);
}
