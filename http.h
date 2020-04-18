#pragma once
#include <objectsockets.h>
#include <string>
#include <regex>

enum class RequestType{
    UNKNOWN,
    GET,
    POST,
    HEAD
};

struct RequestData{
    RequestData();
    ~RequestData();
    std::string* fileName;
    RequestType requestType;
    std::string* requestBody;
    std::string* ext;
    bool dynamic;
    bool keepAlive;
};

class HttpParser{
public:
    static RequestData* parseRequest(const std::string& reuest, std::string& index);
private:
    void FileNameParsing(const std::string& request, RequestData& data);
    void RequestBodyParsing(const std::string& request, RequestData& data);
    void extensionParsing(RequestData& data);
};


class HttpClient{
public:
    HttpClient(const SocketClient& client, const std::string& path);
    bool Send(const RequestData& data);
private:
    SocketClient client;
    std::string path;
    void SendHeader(const RequestData &data, unsigned long size);
    const std::string head = "HTTP/1.1 200 OK\r\n";
    const std::string contenTypeHtml = "Content-Type: text/html; charset=utf-8\r\n";
    const std::string contenTypeJs = "Content-Type: text/javascript\r\n";
    const std::string contenTypeCSS = "Content-Type: text/css\r\n";
    const std::string contenTypeJPG = "Content-Type: image/jpeg\r\n";
    const std::string contenTypePNG = "Content-Type: image/apng\r\n";
    const std::string keepAlive = "Connection: keep-alive\r\n";
    const std::string delimiter = "\r\n";
    const std::string error404 = "HTTP/1.1 404 \r\n\r\n";
    const std::string contentLenght = "Content-Lenght: ";
};
