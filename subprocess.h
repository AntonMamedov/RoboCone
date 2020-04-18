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
class SubProcess
{
public:
    SubProcess(std::function<void()> func);
    SubProcess(std::string path);
    ~SubProcess();
    bool Send(const char* buf, int size);
    int Receive(char* buf);
    int GetInDescriptor();
private:
    int fdin;
    int fdout;
    void comparse(std::string& com, std::vector<char*>& wordList);
};

