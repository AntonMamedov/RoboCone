#pragma once
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <regex>
#include <map>
#include <vector>
#include <list>


struct ConfigData{
    int mainProcessFd;
    std::string index;
    std::string path;
    unsigned PORT;
    bool vebCamStream;
    std::vector<std::pair<std::string, std::vector<char>>>* comPortsInit;
    std::vector<std::pair<std::string, std::string>>* nonPermanentProcesses;
    std::vector<std::pair<std::string, std::string>>* permanentProcesses;
    ConfigData();
    ~ConfigData();
};



/*class ProcessManager{
public:
    ProcessManager(ConfigData& data, std::map<std::string, int>& processRegistr);
    ~ProcessManager();
    bool clientRegistration(std::string processName, int clietnFd);
private:

    std::list<int> epollFdList;
};*/

class ConeConfigParser{
public:
    static void WSConfigParse(std::string config, ConfigData &data);

private:
    std::string BlockFind(const std::string& buf, const std::string& block);
    void PORTParse(const std::string& buf, ConfigData& data);
    void NonPermanentProcessesParse(const std::string& buf, ConfigData &data);
    void PermanentProcessesParse(const std::string& buf, ConfigData &data);
    void MainProcessParse(const std::string& buf, ConfigData &data);
    void VebCamStreamParse(const std::string& buf, ConfigData &data);
    void ComPortsParse(const std::string& buf,  ConfigData &data);
};
