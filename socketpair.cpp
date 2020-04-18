#include "socketpair.h"
SocketPair::SocketPair(std::function<void(SocketPair& socket)> func, StreamMode mode){

     int temp[2];
     socketpair(AF_LOCAL, SOCK_STREAM, 0, temp);
     pid_t process = fork();
     if (process < 0){
         close(temp[0]);
         close(temp[1]);
         throw "Error fork";
     }
     else if(process == 0){
         close(temp[0]);
         socket = dup(temp[1]);
         close(temp[1]);
         if (mode == StreamMode::nonBlocking){
             fcntl(socket, F_SETFL, O_NONBLOCK);
             this->mode = mode;
         }
         func(*this);
     }
     else{
         close(temp[1]);
         socket = dup(temp[0]);
         close(temp[0]);
         if (mode == StreamMode::nonBlocking){
             fcntl(socket, F_SETFL, O_NONBLOCK);
             this->mode = mode;
         }
     }
}
SocketPair::~SocketPair(){
    close(socket);
}

bool SocketPair::Send(const std::string& buf){
    if (write(socket, buf.c_str(), buf.length()) > 0)
        return true;
    else
        return false;
}


bool SocketPair::Send(const char* buf){
    if (write(socket, buf, strlen(buf)) > 0)
        return true;
    else
        return false;

}

std::string SocketPair::Receive(){
    char buf[256];
    int size = read(socket, buf, 256);
    buf[size] = '\0';
    std::string result = buf;
    return buf;
}

int SocketPair::Receive(char *buf){
    int size = read(socket, buf, 400000);
    return size;
}


bool SocketPair::Send(const unsigned char *buf, int size){
    if (write(socket, buf, size) > 0)
        return true;
    else
        return false;
}





bool SocketPair::SendDescriptor(int fd){
    struct msghdr msg = { 0 };
    char buf[CMSG_SPACE(sizeof(fd))];
    memset(buf, '\0', sizeof(buf));
    struct iovec io = { .iov_base = (void*)"ABC", .iov_len = 3 };

    msg.msg_iov = &io;
    msg.msg_iovlen = 1;
    msg.msg_control = buf;
    msg.msg_controllen = sizeof(buf);

    struct cmsghdr * cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    cmsg->cmsg_len = CMSG_LEN(sizeof(fd));

    *((int *) CMSG_DATA(cmsg)) = fd;

    msg.msg_controllen = CMSG_SPACE(sizeof(fd));

    if (sendmsg(socket, &msg, 0))
        return true;
    return false;
}


int SocketPair::ReceiveDescriptor(){
    struct msghdr msg = {0};

    char m_buffer[256];
    struct iovec io = { .iov_base = m_buffer, .iov_len = sizeof(m_buffer) };
    msg.msg_iov = &io;
    msg.msg_iovlen = 1;

    char c_buffer[256];
    msg.msg_control = c_buffer;
    msg.msg_controllen = sizeof(c_buffer);

    recvmsg(socket, &msg, 0) < 0;
    struct cmsghdr * cmsg = CMSG_FIRSTHDR(&msg);

    unsigned char * data = CMSG_DATA(cmsg);


    unsigned fd = *((unsigned*) data);

    return fd;
}


int SocketPair::GetDescriptor(){
    return socket;
}


bool SocketPair::operator==(const SocketPair &other){
    return socket == other.socket;
}


