#pragma once
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <functional>
#include <vector>
#include <unistd.h>
#include <string.h>
#include <vector>
#include <WebConst.h>

class SocketPair
{
public:
    SocketPair(std::function<void(SocketPair& socket)> func, StreamMode flag = StreamMode::blocking);
    SocketPair(std::string path);
    ~SocketPair();
    bool Send(const std::string& buf);
    bool Send(const char* buf);
    bool Send(const unsigned char* buf, int size);
    bool SendDescriptor(int fd);
    std::string Receive();
    int Receive(char* buf);
    int ReceiveDescriptor();
    int GetDescriptor();
    bool operator==(const SocketPair& other);

private:
    int socket;
    StreamMode mode;
};
