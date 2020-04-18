#pragma once
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <functional>
#include <WebConst.h>
const unsigned int maxConnaction = 10000;
class SocketClient{
public:
    SocketClient(int serverDescriptor, bool block = true);
    SocketClient(const SocketClient &other);
    SocketClient(int* fd);
    SocketClient();
    ~SocketClient();
    bool SendAll(const std::string& buf);
    bool SendAll(const char* buf, int size);
    bool SendAll(const unsigned char* buf, unsigned long size);
    bool SendFile(std::string path);
    bool SendFile(int fd);
    char* Receive();
    int GetSocketDescriptor();
    SocketClient& operator=(const SocketClient& other);
private:
    int socket_;
};



class SocketServer{
public:
    SocketServer(std::string PORT, unsigned int maxConaction = maxConnaction, bool block = true);
    ~SocketServer();
    int GetSocketDescriptor();
    SocketClient* Listen(bool block = true);
    friend bool operator==(const SocketServer& th, const SocketServer& other);
    friend bool operator!=(const SocketServer& th, const SocketServer& other);
private:
    int socket_;
    void sortAddr(addrinfo *servinfo);

};
