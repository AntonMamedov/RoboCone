#pragma once
#include <objectsockets.h>
#include <WebConst.h>
#include "sha1base64.h"



#define MAX_RECV_BUF 65535
#define TXT 0x01
#define BIN 0x02
#define CLOSE 0x08
#define PING 0x09
#define PONG 0x0A


struct WebSocketMessage{
    int size;
    int opcode;
    int fin;
    char* message;
    WebSocketMessage();
    ~WebSocketMessage();
};


class WebSocket
{
public:
    WebSocket(SocketClient client);
    WebSocket(const WebSocket& other);
    bool Send(const std::string& buf, int opcode);
    bool Send( char* buf, int opcode, unsigned long size);
    bool Send(const unsigned char* buf, unsigned long size, int opcode);
    void Resive(WebSocketMessage& message);
    int GetDescriptor();
private:
    SocketClient client;
    int ParsePayloadSiezeLenght(char seckondByte);
    unsigned long ParseBodySize(char* bodyLenght, int size);
    void BodyDecode(char* payload, const char* mask, int payloadSize);
    void PayloadSize(char* frame, unsigned long size, int frameSize);
};


class WebSocketOpener{
public:
    static void OpenWebSocketConnection(char* request, int clientDescriptor);
private:
    char* ParseWebSocketKey(char* reuest);
    void WebSocketOpen(int clietn_d, char* key);
};
