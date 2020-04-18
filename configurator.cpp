#include "configurator.h"
#include <sys/epoll.h>
ConfigData::ConfigData(){
    comPortsInit = nullptr;
    nonPermanentProcesses = nullptr;
    permanentProcesses = nullptr;
}

ConfigData::~ConfigData(){
    delete comPortsInit;
    delete nonPermanentProcesses;
    delete permanentProcesses;
}




void ConeConfigParser::VebCamStreamParse(const std::string &buf, ConfigData &data){
    std::cmatch result;
    std::regex regular("(VebCamStream) (\\s*=\\s*) ([\\w]*)");
    if (std::regex_search(buf.c_str(), result, regular)){
        if (result[3] == "on")
            data.vebCamStream = true;
        else
            data.vebCamStream = false;
    }
    else{
        throw "Error web cam stream parse";
    }
}


void ConeConfigParser::ComPortsParse(const std::string& buf,  ConfigData &data){
    std::cmatch result;
    std::regex regular("(ComPortsControl) (\\s*=\\s*) ([\\w]*)");
    if (std::regex_search(buf.c_str(), result, regular)){
        if (result[3] == "on"){
               std::string str = BlockFind(buf, "ComPortsConfig");
               std::cout << str << std::endl;
               std::regex date_pat1{"([\\w\\d-_/\.]+)([\\s]*=[\\s]*)([{\\w\\d\\s,<>=}]+[^;]+)"};
               std::vector<std::string> res;
               std::vector<std::pair<std::string, std::string>> buffer;
               for(std::sregex_iterator i = std::sregex_iterator(str.begin(), str.end(), date_pat1);
                                         i != std::sregex_iterator();
                                         ++i)
                {
                    if (data.comPortsInit == nullptr)
                        data.comPortsInit = new std::vector<std::pair<std::string, std::vector<char>>>;
                    std::smatch m = *i;
                    std::pair<std::string, std::string> temp(m[1],m[3]);
                    buffer.push_back(temp);
                }
               for (auto it = buffer.begin(); it != buffer.end(); it++){
                       std::string name = it->first;

                       char prev = '1';
                       std::vector<char> byts;
                       for (auto i = it->second.begin(); i != it->second.end(); i++){
                           char next = *(i + 1);
                           if ((*i != '{' ) && (*i != '}') && (*i != ',' )){
                               if (prev == ',' && next == ',' && *i == ' '){
                                   char test = *i;
                                   byts.push_back( *i);

                               }
                               else if ((prev != ',' || next != ',') && *i == ' '){
                                   prev = *i;
                                   continue;
                               }
                               else{
                                   char test = *i;
                                   byts.push_back( *i);
                               }
                           }
                           prev = *i;
                       }
                   std::pair<std::string, std::vector<char>> node(name, byts);
                   data.comPortsInit->push_back(node);

               }
        }
        else
            return;
    }
    else{
        throw "Error com port parse";
    }

}

void ConeConfigParser::WSConfigParse(std::string config, ConfigData &data){
    int configFd = open(config.c_str(), O_RDONLY);
    if (configFd < 0)
    {
        throw "File not found";
    }
    else
    {
        struct stat st;
        fstat(configFd, &st);
        char* buf = new char[st.st_size];
        if (read(configFd, buf, st.st_size))
        {
            ConeConfigParser temp;
            std::string WSServerBuf = temp.BlockFind(buf, "WSServer");
            delete [] buf;
            temp.PORTParse(WSServerBuf, data);
            temp.MainProcessParse(WSServerBuf, data);
            temp.PermanentProcessesParse(WSServerBuf, data);
        }
        else
        {
             delete[] buf;
             throw "WebSocket configuration not specified\n";
        }
        close(configFd);

    }
}

void ConeConfigParser::MainProcessParse(const std::string& buf, ConfigData &data){

       std::string temp = BlockFind(buf, "MainProcessConfig");
       VebCamStreamParse(temp, data);
       ComPortsParse(temp, data);
}

std::string ConeConfigParser::BlockFind(const std::string& buf, const std::string& block){
       auto start = buf.find(block);
       if (!start){
           throw "Block not found";
       }
       std::string temp =  buf.substr( start, buf.size() - start);
       start = temp.find('{') + 1;
       auto end = start;
       int count = 1;
       for (; end < buf.size(); end++){
           if (temp[end] == '{')
               count++;
           else if (temp[end] == '}')
               count--;
           if (count ==0)
               break;
       }
       end--;
       return  temp.substr(start, end - start);
}



void ConeConfigParser::PORTParse(const std::string &buf, ConfigData& data){
    std::cmatch result;
    std::regex regular("(PORT) (\\s*=\\s*) (\\d*)");
    if (std::regex_search(buf.c_str(), result, regular))
    {
        std::string num = result[3];
        data.PORT = atoi(num.c_str());
    }
    else{
        throw "Port configuration not found";
    }
}


void ConeConfigParser::PermanentProcessesParse(const std::string& buf, ConfigData &data){
    std::cmatch result;
    std::regex regular("(PermanentProcessesConfig)([\\s*{\\s*])([.]*[^}]+)(})");
    if (std::regex_search(buf.c_str(), result, regular)){
        std::regex date_pat1{"([\\w\\d-_/\.]+)([\\s]*=[\\s]*)([\\w/\._-]+[^;]*)"};
            std::string s = result[3];
            std::vector<std::string> res;

            for(std::sregex_iterator i = std::sregex_iterator(s.begin(), s.end(), date_pat1);
                                     i != std::sregex_iterator();
                                     ++i)
            {
                if (data.permanentProcesses == nullptr)
                    data.permanentProcesses = new std::vector<std::pair<std::string, std::string>>;
                std::smatch m = *i;
                std::string j = m[1];
                std::string j1 =m[3];
                std::pair<std::string, std::string> temp(m[1],m[3]);
                data.permanentProcesses->push_back(temp);
            }
    }
    else
    {
        throw "PermanentProcesses configuration not found";
    }

}


void ConeConfigParser::NonPermanentProcessesParse(const std::string& buf, ConfigData &data)
{
    std::cmatch result;
    std::regex regular("(NonPermanentProcessesConfig)([\\s*{\\s*])([.]*[^}]+)(})");
    if (std::regex_search(buf.c_str(), result, regular)){
        std::regex date_pat1{"([\\w\\d-_/\.]+)([\\s]*=[\\s]*)([\\w/\._-]+[^;]*)"};
            std::string s = result[3];
            std::vector<std::string> res;

            for(std::sregex_iterator i = std::sregex_iterator(s.begin(), s.end(), date_pat1);
                                     i != std::sregex_iterator();
                                     ++i)
            {
                if (data.nonPermanentProcesses == nullptr)
                    data.nonPermanentProcesses = new std::vector<std::pair<std::string, std::string>>;
                std::smatch m = *i;
                std::pair<std::string, std::string> temp(m[1],m[3]);
                data.nonPermanentProcesses->push_back(temp);
            }
    }
    else
    {
        throw "NonPermanentProcesses configuration not found";
    }

}


