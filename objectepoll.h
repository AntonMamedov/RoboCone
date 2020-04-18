#pragma once
#include <sys/epoll.h>
#include <unistd.h>
class ObjectEpoll
{
public:
    ObjectEpoll();
    ~ObjectEpoll();
    int Wait(epoll_event* events, int maxEvents, int timeOut = -1);
    void AddDescriptor(int fd, uint32_t flags, void* ptr = nullptr);
private:
    int epollDescriptor;
};

