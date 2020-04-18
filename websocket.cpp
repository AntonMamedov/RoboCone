#include "websocket.h"

WebSocketMessage::WebSocketMessage(){
    fin = 0;
    opcode = 0;
    message = nullptr;
}


WebSocketMessage::~WebSocketMessage(){
    delete message;
}


WebSocket::WebSocket(SocketClient client)
{
    this->client = client;
}

WebSocket::WebSocket(const WebSocket& other){
    this->client = other.client;
}


int WebSocket::GetDescriptor(){
    return client.GetSocketDescriptor();
}




void  WebSocket::Resive(WebSocketMessage& message){
    //читаем первые 2 байта фрейма
    char frame_start[2];
    int fd = recv(client.GetSocketDescriptor(), frame_start, 2, 0);
    //парсим первые 2 байта фрейма
    message.fin = frame_start[0] & 0x80; // определяем фрагментировано ли сообщение
    message.fin >>=4;
    message.opcode = frame_start[0] & 0x0f; // определяем опкод
    if (message.opcode == CLOSE)
    {
        return;
    }
    //определяем в скольких байтах записан размер данных фрейма
    int payloadSizeLenght = ParsePayloadSiezeLenght(frame_start[1]);
    // если данные меньше 126 и их длина хранится во втором байте
    if (payloadSizeLenght == 0)
    {
        char mask[4];
        recv(client.GetSocketDescriptor(), mask, 4, 0);
        int payloadLenght = frame_start[1] & 0x7f;
        char* payload = (char*)malloc(sizeof (char) * payloadLenght + 1);
        payload[payloadLenght] = '\0';
        message.size = payloadLenght;
        recv(client.GetSocketDescriptor(), payload, payloadLenght, 0);
        BodyDecode(payload, mask, payloadLenght);
        message.message = payload;
        return;
    }
    //если данные больше 126 байт
    else if (payloadSizeLenght  == 2)
    {
        char payload_size_frame[2];
        recv(client.GetSocketDescriptor(), payload_size_frame, 2, 0);
        int payload_size = ParseBodySize(payload_size_frame, 2);
        char mask[4];
        recv(client.GetSocketDescriptor(), payload_size_frame, 4, 0);
        char* payload = (char*)malloc(sizeof(char) * payload_size + 1);
        payload[payload_size] = '\0';
        message.size = payload_size;
        recv(client.GetSocketDescriptor(), payload, payload_size, 0);
        BodyDecode(payload, mask, payload_size);
        message.message = payload;
        return;

    }
    else if (payloadSizeLenght == 8)
    {
        char payload_size_frame[8];
        recv(client.GetSocketDescriptor(), payload_size_frame, 8, 0);
        int payload_size = ParseBodySize(payload_size_frame, 2);
        char mask[4];
        recv(client.GetSocketDescriptor(), payload_size_frame, 4, 0);
        char* payload = (char*)malloc(sizeof(char) * payload_size + 1);
        payload[payload_size] = '\0';
        int shift = 0;
        while (shift != payload_size)
        {
            int read = recv(client.GetSocketDescriptor(), payload + shift, 30000, 0);
            shift+=read;
        }
        BodyDecode(payload, mask, payload_size);
        message.message = payload;
        message.size = payload_size;
        return;
    }

}


int WebSocket::ParsePayloadSiezeLenght(char seckondByte){
    if ((seckondByte & 0x7f) < 126)
        return 0;
    else if ((seckondByte & 0x7f) == 126)
        return 2;
    else if ((seckondByte & 0x7f) == 127)
        return 8;
}


unsigned long WebSocket::ParseBodySize(char* bodyLenght, int size){
    unsigned long buf;
    if (size == 8){
        buf = bodyLenght[0] * 10000000 + bodyLenght[1] * 1000000 + bodyLenght[2]
                * 100000 + bodyLenght[3] * 10000 + bodyLenght[4] * 1000 + bodyLenght[5]
                * 100 + bodyLenght[6] * 10 + bodyLenght[7];
    }
    else if (size == 2)
        buf = bodyLenght[0] * 10 + bodyLenght[1];
    return buf;
}


void WebSocket::BodyDecode(char* payload, const char* mask, int payloadSize){
    for (int i = 0; i < payloadSize; i++)
        payload[i] = payload[i]^mask[i % 4];
}


void WebSocket::PayloadSize(char* frame, unsigned long size, int frameSize){
    for (ssize_t i = 2; i < frameSize; i++)
        frame[i] = 0;
    for (int i = frameSize - 1; size != 0; i--)
    {
        frame[i] = size & 0xFF;
        size >>= 8;
    }

}


bool WebSocket::Send(const std::string &buf, int opcode){
    auto size = buf.length();
    if (size < 126)
    {
        char frame[2];
        frame[0] = 8;
        frame[0] <<=4;
        frame[0] = frame[0] | opcode;
        frame[1] = size;
        client.SendAll(frame);
    }
    else if ( size <= maxReceiveBufferSize)
    {
        char frame[4];
        frame[0] = 8;
        frame[0] <<=4;
        frame[0] = frame[0] | opcode;
        frame[1] = 126;
        PayloadSize(frame, size, 4);
        client.SendAll(frame);
    }
    else if(size > maxReceiveBufferSize)
    {
        char frame[10];
        frame[0] = 8;
        frame[0] <<=4;
        frame[0] = frame[0] | opcode;
        frame[1] = 127;
        PayloadSize(frame, size, 10);
        client.SendAll(frame);
    }
    return client.SendAll(buf);
}


bool WebSocket::Send(char* buf, int opcode, unsigned long size){
    if (size < 126)
    {
        char frame[2];
        frame[0] = 8;
        frame[0] <<=4;
        frame[0] = frame[0] | opcode;
        frame[1] = size;
        client.SendAll(frame, 2);
    }
    else if ( size <= maxReceiveBufferSize)
    {
        char frame[4];
        frame[0] = 8;
        frame[0] <<=4;
        frame[0] = frame[0] | opcode;
        frame[1] = 126;
        PayloadSize(frame, size, 4);
        client.SendAll(frame, 4);
    }
    else if(size > maxReceiveBufferSize)
    {
        char frame[10];
        frame[0] = 8;
        frame[0] <<=4;
        frame[0] = frame[0] | opcode;
        frame[1] = 127;
        PayloadSize(frame, size, 10);
        client.SendAll(frame, 10);
    }
    return client.SendAll(buf, size);
}


bool WebSocket::Send(const unsigned char* buf, unsigned long size, int opcode){
    if (size < 126)
    {
        char frame[2];
        frame[0] = 8;
        frame[0] <<=4;
        frame[0] = frame[0] | opcode;
        frame[1] = size;
        client.SendAll(frame, 2);
    }
    else if ( size <= maxReceiveBufferSize)
    {
        char frame[4];
        frame[0] = 8;
        frame[0] <<=4;
        frame[0] = frame[0] | opcode;
        frame[1] = 126;
        PayloadSize(frame, size, 4);
        client.SendAll(frame, 4);
    }
    else if(size > maxReceiveBufferSize)
    {
        char frame[10];
        frame[0] = 8;
        frame[0] <<=4;
        frame[0] = frame[0] | opcode;
        frame[1] = 127;
        PayloadSize(frame, size, 10);
        client.SendAll(frame, 10);
    }
    return client.SendAll(buf, size);
}


/*********************/


void WebSocketOpener::OpenWebSocketConnection(char* request, int clientDescriptor){
    WebSocketOpener opener;
    char* key = opener.ParseWebSocketKey(request);
    opener.WebSocketOpen(clientDescriptor, key);
    free(key);
}


char* WebSocketOpener::ParseWebSocketKey(char* request){
    char *point;
    if ((point = strstr(request, "Sec-WebSocket-Key:") ) != NULL)
    {
        char* resultstr =(char*) malloc(sizeof (char) * 64);
        int i = 0, it = 0;
        for(i = 19; it < 24; i++, it++)
            resultstr[it] = point[i];
        resultstr[it] = '\0';
        strcat(resultstr, GUIDKey);
        return resultstr;
    }
    else
        return nullptr;
}



void WebSocketOpener::WebSocketOpen(int clietnDesriptor, char* key){
    int fd = write(clietnDesriptor, "HTTP/1.1 101 Switching Protocols\r\n", strlen("HTTP/1.1 101 Switching Protocols\r\n"));
    int fd1 = write(clietnDesriptor, "Upgrade: WebSocket\r\n",strlen("Upgrade: WebSocket\r\n"));
    int fd2 = write(clietnDesriptor, "Connection: Upgrade\r\n", strlen("Connection: Upgrade\r\n"));
    unsigned char temp[20];
    SHA1(temp, key, strlen(key));
    unsigned char key_out[64] = {0,};
    int size = base64_encode(temp, key_out, sizeof (temp));
    int fd3 =write(clietnDesriptor, "Sec-WebSocket-Accept: ", strlen("Sec-WebSocket-Accept: "));
   int fd4 =write(clietnDesriptor, key_out, size);
   int fd5 =write(clietnDesriptor, "\r\n\r\n", strlen("\r\n\r\n"));
}
