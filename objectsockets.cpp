#include "objectsockets.h"



SocketServer::SocketServer(std::string PORT, unsigned int maxConaction, bool block){
    struct addrinfo hints;
    struct addrinfo *servinfo;

    memset(&hints, 0, sizeof(hints));

    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags    = AI_PASSIVE;

    int r = getaddrinfo(NULL, PORT.c_str(), &hints, &servinfo);
    if( r != 0){
        throw "Error getaddrinfo";
    }
    sortAddr(servinfo);
    freeaddrinfo(servinfo);

    if(listen(socket_, maxConaction) == -1){
      throw "Erro listen";
    }
    if (block == false)
        fcntl(socket_, F_SETFL, O_NONBLOCK);
}


SocketServer::~SocketServer(){
    close(socket_);
}


void SocketServer::sortAddr(addrinfo *servinfo){
    struct addrinfo *p;
    int sock = 0;
    int yes;
    for(p = servinfo; p != NULL; p = p->ai_next)
    {
      socket_ = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
      if(socket_ == -1)
        continue;

      if (setsockopt( socket_, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
      {
        close(socket_);
        freeaddrinfo(servinfo);
        throw "Error setsockopt";
      }

      if(bind(socket_, p->ai_addr, p->ai_addrlen) == -1)
      {
        close(sock);
        continue;
      }
      break;
    }
    freeaddrinfo(servinfo);
    if(p == NULL){
      throw "Error bind";
    }

}


int SocketServer::GetSocketDescriptor(){
    return socket_;
}


SocketClient* SocketServer::Listen(bool block){
    SocketClient* client = new SocketClient(socket_, block);
    return client;
}


bool operator==(const SocketServer& th, const SocketServer& other){
    return th.socket_ == other.socket_;
}


bool operator!=(const SocketServer& th, const SocketServer& other){
    return th.socket_ != other.socket_;
}


/**************************************************************************************************/


SocketClient::SocketClient(int serverDescriptor, bool block){
    sockaddr_storage client_addr;
    socklen_t s_size = sizeof(client_addr);
    socket_ = accept(serverDescriptor, (sockaddr*)&client_addr, &s_size);
    if (block == false)
        fcntl(socket_, F_SETFL, O_NONBLOCK);
}


SocketClient::SocketClient(const SocketClient &other){
    this->socket_ = dup(other.socket_);
}

SocketClient::SocketClient(int* fd){
    this->socket_ = dup(*fd);
}

SocketClient::SocketClient(){
    socket_ = false;
}


SocketClient::~SocketClient(){
    close(socket_);
}


int SocketClient::GetSocketDescriptor(){
    return socket_;
}


bool SocketClient::SendAll(const std::string& buf){
    auto size = buf.length();
    ssize_t shift = 0;
    while (shift < size){
        auto sizeSend = send(socket_, buf.c_str() + shift, size, 0);
        if (sizeSend == -1)
            return false;
        shift+= sizeSend;
    }
    return true;
}


bool SocketClient::SendAll(const char *buf, int size){
    ssize_t shift = 0;
    while (shift < size){
        auto sizeSend = send(socket_, buf + shift, size, 0);
        if (sizeSend == -1)
            return false;
        shift+= sizeSend;
    }
    return true;
}


bool SocketClient::SendAll(const unsigned char* buf, unsigned long size){
    ssize_t shift = 0;
    while (shift < size){
        int sizeSend = send(socket_, buf + shift, size, 0);
        if (sizeSend == -1)
            return false;
        shift+= sizeSend;
    }
    return true;
}


bool SocketClient::SendFile(std::string path){
    int fd = open(path.c_str(), O_RDONLY);
    if (fd > 0)
    {
        struct stat st;
        fstat(fd, &st);
        off_t offset = 0;
        sendfile (socket_, fd, &offset, st.st_size);
        close(fd);
        return true;
    }
    else
    {
        close(fd);
        return false;
    }
}


bool SocketClient::SendFile(int fd){
    if (fd > 0)
    {
        struct stat st;
        fstat(fd, &st);
        off_t offset = 0;
        sendfile (socket_, fd, &offset, st.st_size);
        close(fd);
        return true;
    }
    else
    {
        close(fd); //проверить
        return false;
    }
}


char* SocketClient::Receive(){
    char buf[maxReceiveBufferSize];
    if (socket_ == false)
        return nullptr;
    auto size = recv(socket_, buf, maxReceiveBufferSize, 0);
    if (size < 0)
        return nullptr;
    else{
        buf[size] = '\0';
        char* result = buf;
        return result;
    }
}


 SocketClient& SocketClient::operator=(const SocketClient& other){
    this->socket_ = dup(other.socket_);
     return *this;
}


