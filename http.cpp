#include "http.h"


RequestData::RequestData(){
    fileName = nullptr;
    requestBody = nullptr;
    dynamic = false;
    keepAlive = false;
}


RequestData::~RequestData(){
    delete fileName;
    delete requestBody;
}


RequestData* HttpParser::parseRequest(const std::string& request, std::string& index){
    RequestData* data = new RequestData;
    HttpParser temp;
    if (request[0] == 'G'&& request[1] == 'E' && request[2] == 'T'){
        data->requestType = RequestType::GET;
        temp.FileNameParsing(request, *data);
        if (data->fileName == nullptr){
            data->fileName = new std::string;
            *data->fileName = index;
        }
        temp.extensionParsing(*data);
        if (data->dynamic == true)
          temp.RequestBodyParsing(request, *data);
    }
    else if (request[0] == 'P'&& request[1] == 'O' && request[2] == 'S' && request[3] == 'T'){
        data->requestType = RequestType::POST;
        temp.FileNameParsing(request, *data);
        temp.extensionParsing(*data);
        temp.RequestBodyParsing(request, *data);
    }
    else if (request[0] == 'H'&& request[1] == 'E' && request[2] == 'A' && request[3] == 'D'){
        data->requestType = RequestType::HEAD;

    }
    else
        return nullptr;
    return data;
}


void HttpParser::FileNameParsing(const std::string &request, RequestData &data){
    char buf[1024];
    auto count = request.find(' ') + 1;
    if (request[count + 2] == 'H' && request[count + 3] == 'T' && request[count + 4] == 'T' && request[count + 5] == 'P')
        return;
    auto shift = count;
    while(request[count] != ' '){
        buf[count - shift] = request[count];
      //  std::cout << buf[count - shift] << std::endl;
        count++;
    }
    buf[count - shift] = '\0';
    data.fileName = new std::string;
    *data.fileName = buf;
    if (request[count] == '?')
        data.dynamic = true;
}


void HttpParser::extensionParsing(RequestData& data){
    if (data.fileName != nullptr){
        auto count = data.fileName->find('.');
        if (!count)
            return;
        else
        {
            data.ext = new std::string;
            while (count < data.fileName->length()){
                data.ext->push_back(data.fileName->operator[](count));
                count++;
            }
        }
    }
}


void HttpParser::RequestBodyParsing(const std::string& request, RequestData& data){
    if (data.requestType == RequestType::GET){
        auto count = request.find('?');
        data.requestBody = new std::string;
        for (; request[count] != ' '; count++)
            data.requestBody->push_back(request[count]);
    }
    else{
        std::string key = "Content-Length:";
        auto start = request.find(key) + key.length();
        if (!start)
            return;
        data.requestBody = new std::string;
        auto end = start;
        for (; request[end] != '\r'; end++);
        std::string Content_lenght = request.substr(start + 1, end - start);
        if (atoi(Content_lenght.c_str()) > 0)
            *data.requestBody = request.substr(request.length() - atoi(Content_lenght.c_str()), request.length());
    }
}


/**************************************/


HttpClient::HttpClient(const SocketClient& client,const std::string& path){
    this->client = client;
    this->path = path;
}


bool HttpClient::Send(const RequestData &data){
    struct stat st;
    stat(path.c_str(), &st);
    SendHeader(data, st.st_size);
    return client.SendFile(path + *data.fileName);
}


void HttpClient::SendHeader(const RequestData &data, unsigned long size){
    if (data.fileName != nullptr){
        if (*data.ext == ".html"){
            client.SendAll(head);
            client.SendAll(contenTypeHtml);

        }
        else if (*data.ext == ".js"){
            client.SendAll(head);
            client.SendAll(contenTypeJs);
        }
        else if (*data.ext == ".css"){
            client.SendAll(head);
            client.SendAll(contenTypeCSS);
        }
        else if (*data.ext == ".jpg" || *data.ext == ".jpeg"){
            client.SendAll(head);
            client.SendAll(contenTypeJPG);
        }
        else if (*data.ext == ".png" || *data.ext == ".gif"){
            client.SendAll(head);
            client.SendAll(contenTypePNG);
        }
        else{
            client.SendAll(error404);
        }
        client.SendAll(contentLenght + std::to_string(size) + delimiter);
        client.SendAll(delimiter);
    }


}
