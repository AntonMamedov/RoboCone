#include <iostream>
#include <objectepoll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <vector>
#include <http.h>
#include <signal.h>
#include "socketpair.h"
#include <websocket.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <subprocess.h>
#include <thread>
#include <sys/resource.h>
#include <configurator.h>
#include <ardoinocomport.h>
#include <map>
#include <string>
#include <mutex>
#include <tuple>
#include <list>
using namespace std;



int main(){
   signal ( SIGPIPE, SIG_IGN );
// открываем процесс для http сервера
    if (fork() == 0){
        std::string path = "/home/anton/Programming/";
        std::string index = "index.html";
        ObjectEpoll eventsManager;
        SocketServer server("8000", false);
        eventsManager.AddDescriptor(server.GetSocketDescriptor(), EPOLLIN, &server);
        epoll_event events[1000];
        while(1){
            auto size = eventsManager.Wait(events, 1000);
            for (int i = 0; i < size; i++){
                if (events[i].data.ptr == &server){

                    SocketClient* client = server.Listen(false);
                    eventsManager.AddDescriptor(client->GetSocketDescriptor(), EPOLLIN | EPOLLET, client);
                }
                else{
                    SocketClient *client = (SocketClient*)events[i].data.ptr;
                    const char* request = client->Receive();
                    if (request == nullptr){
                        delete client;
                        continue;
                    }
                    RequestData* data = HttpParser::parseRequest(request, index);
                    HttpClient httpClient(*client, path);
                    httpClient.Send(*data);
                    delete data;
                    delete client;
                }
            }
        }
     }



    //парсим конфиг
    ConfigData* data = new ConfigData;
    ConeConfigParser::WSConfigParse("/home/anton/regtest", *data);
    //открываем мэин процесс (вебка и компорты)
    SocketPair mainProcess([&](SocketPair parent){
        rlimit test;
        getrlimit(RLIMIT_NOFILE, &test);
        test.rlim_cur = 4050;
        setrlimit(RLIMIT_NOFILE, &test);
        CvCapture* capture = cvCreateCameraCapture(CV_CAP_ANY);
        IplImage* frame= nullptr;
        ObjectEpoll epOut;
        ObjectEpoll epIn;
        epIn.AddDescriptor(parent.GetDescriptor(), EPOLLIN | EPOLLET, &parent);
        std::map<char, ArduinoComPort*> portManager;
        for (auto it = data->comPortsInit->begin(); it != data->comPortsInit->end(); it++){
            ArduinoComPort* port = nullptr;
            try {
                port  = new ArduinoComPort(it->first);
            } catch (const std::exception& ex) {
                delete port;
                cout << it->first << " :  ";
                cout << ex.what() << std::endl;
                continue;
            }
            for (auto i = it->second.begin(); i != it->second.end(); i++){
                std::pair<char, ArduinoComPort*> node(*i, port);
                portManager.emplace(node);
            }
        }
        //обрабатываем пришедшие от клиента сообщения и пришедшие из оснвного процесса дескрипторы
        thread heandler([&](){
            while(1){
                epoll_event events[20];
                int size = epIn.Wait(events, 20);
                for (int i = 0; i < size; i++){
                    if (events[i].data.ptr == &parent){
                        int client = parent.ReceiveDescriptor();
                        WebSocket* StreamWs = new WebSocket(&client);
                        close(client);
                        epIn.AddDescriptor(StreamWs->GetDescriptor(), EPOLLIN | EPOLLET, StreamWs);
                        epOut.AddDescriptor(StreamWs->GetDescriptor(), EPOLLOUT, StreamWs);
                    }
                    else{
                         WebSocket* StreamWs = (WebSocket*)events[i].data.ptr;
                         WebSocketMessage msg;
                         StreamWs->Resive(msg);
                         if (msg.opcode == CLOSE){
                             delete StreamWs;
                             continue;
                         }
                         auto it = portManager.find(msg.message[0]);
                         if (it != portManager.end())
                            it->second->Send(msg.message, 1);
                    }
                }
            }

        });

        heandler.detach();
        //в цикле ждем epoll_wait, когда приходит клиент начинаем отдавать ему кадры
        epoll_event events[10];
            while(1)
            {
                int size = epOut.Wait(events, 10);
                frame = cvQueryFrame( capture );
                int j = frame->imageSize;
                auto temp = cvEncodeImage(".jpg", frame);
                int i = 0;
                 uchar *img = temp->data.ptr;
                for (; i < size; i++){
                    WebSocket* cl;
                    cl = (WebSocket*)events[i].data.ptr;
                    cl->Send(img, temp->width, BIN);

                }
                cvReleaseMat(&temp);
            }
            cvReleaseCapture( &capture );
    });

    SocketServer server("8001");
    ObjectEpoll controller;
    pair <int, int> test(4, 0);
    controller.AddDescriptor(0, EPOLLIN, &test);
    //уборщик мусора, будем складывать туда указатели, которые нельзя удалить внутри контекста
    list <void* > garbageCollector;

    std::pair<int, void*>* serverPair = new  std::pair<int, void*>(0, &server);
    garbageCollector.push_back(serverPair);
    controller.AddDescriptor(server.GetSocketDescriptor(), EPOLLIN, serverPair);
    //создаем дерево процессов, в нем хранятся имена процессов, указатели на объекты процессов и вектор связанных с процессом клиентов
    map<std::string, std::pair<SubProcess*, std::vector<WebSocket*>>> procesesTree;
    // инициализация дерва процессов из данных, полученных из конфига
    for (auto it = data->permanentProcesses->begin(); it != data->permanentProcesses->end(); it++){
        std::vector<WebSocket*> vec;
        SubProcess* sub = new SubProcess(it->second);
        pair<std::string, std::pair<SubProcess*, std::vector<WebSocket*>>> elem(it->first, make_pair(sub, vec));
        procesesTree.insert(elem);
        std::string* tempString = new std::string (it->first);
        pair<int, std::string*>* tempPair =  new pair<int,  std::string*> (3, tempString);
        controller.AddDescriptor(sub->GetInDescriptor(), EPOLLIN | EPOLLET, tempPair);
        garbageCollector.push_back(tempPair);
    }
    // удаляем более ненужные данные конфига
    delete data;
    //далее создаем цикл, это реализация автомата, мы будем слушать дескриптор epoll и обрабатывать его сигналы
    epoll_event events[100];
    while(true){
        auto size = controller.Wait(events, 100);
        //обрабатываем все события, пришедшие из epoll
        for (int i = 0; i < size; i++){
            pair<int, void*>* event = (pair<int, void*>*)events[i].data.ptr;
            switch (event->first) {
            //если нам приходит запрос на соедиенение, мы создаем WS соединение и помещаем его в пул событий
            case 0:{
                SocketServer* pServer = (SocketServer*)event->second;
                SocketClient* client = pServer->Listen();
                char* request = client->Receive();
                WebSocketOpener::OpenWebSocketConnection(request, client->GetSocketDescriptor());
                WebSocket* webSocketClient = new WebSocket(*client);
                delete client;
                pair<int, void*>* ev = new pair<int, void*>(1, webSocketClient);
                controller.AddDescriptor(webSocketClient->GetDescriptor(), EPOLLIN | EPOLLONESHOT, ev);
                garbageCollector.push_back(ev);
                break;
            }
            //если сообщение пришло от еще неподписанного на процесс клиента
            case 1:{
                WebSocket* webSocketClient = new WebSocket(*(WebSocket*)event->second);
                delete (WebSocket*)event->second;
                WebSocketMessage msg;
                webSocketClient->Resive(msg);

                if (msg.opcode == CLOSE  ){
                    for (auto it = garbageCollector.begin(); it != garbageCollector.end(); it++){
                        if (*it == webSocketClient){
                           garbageCollector.erase(it);
                           break;
                        }
                    }
                    delete webSocketClient;
                    break;
                }
                const std::string comparator = "main";
                std::string message = msg.message;
                if (message == comparator){
                    mainProcess.SendDescriptor(webSocketClient->GetDescriptor());
                    for (auto it = garbageCollector.begin(); it != garbageCollector.end(); it++){
                        if (*it == webSocketClient){
                           garbageCollector.erase(it);
                           break;
                        }
                    }
                    delete webSocketClient;
                    break;
                }
                else{
                    auto it = procesesTree.find(message);
                    if (it == procesesTree.end()){
                        for (auto it = garbageCollector.begin(); it != garbageCollector.end(); it++){
                            if (*it == webSocketClient){
                               garbageCollector.erase(it);
                               break;
                            }
                        }
                        delete webSocketClient;
                        break;
                    }
                    pair<std::string, WebSocket*>* node = new  pair<std::string, WebSocket*>(msg.message, webSocketClient);
                    pair<int,  pair<std::string, WebSocket*>*>* ev = new  pair<int,  pair<std::string, WebSocket*>*>(2, node);
                    controller.AddDescriptor(webSocketClient->GetDescriptor(), EPOLLIN | EPOLLET, ev);
                    it->second.second.push_back(webSocketClient);
                    garbageCollector.push_back(ev);
                }
                break;
            }
            //если сообщение пришло от подписанного на процесс клиента
            case 2:{
                pair<std::string, WebSocket*>* client = (pair<std::string, WebSocket*>*)event->second;
                std::string name = client->first;
                WebSocket* webSocketClient = client->second;
                WebSocketMessage msg;
                webSocketClient->Resive(msg);
                if (msg.opcode == CLOSE  ){
                    for (auto it = garbageCollector.begin(); it != garbageCollector.end(); it++){
                        if (*it == webSocketClient){
                           garbageCollector.erase(it);
                           break;
                        }
                    }
                    delete webSocketClient;
                    break;
                }
                auto it = procesesTree.find(client->first);
                auto process = it->second.first;
                process->Send(msg.message, msg.size);
                break;

            }
                // если пришло сообщение от процесса
            case 3:{
                std::string* name = (std::string*)event->second;
                const char *c = name->c_str();
                auto it = procesesTree.find(*name);
                char buf[1025];
                int size = it->second.first->Receive(buf);
                vector<WebSocket*>* tempVec = &it->second.second;

                for (int i = 0; i < tempVec->size(); i++){
                    tempVec->operator[](i)->Send(buf, TXT, size);
                }
                break;
            }
                // пришло сообщение из потока ввода
            case 4:{
                char buf[50];
                int size = read(0, buf, 20);
                buf[size] = '\0';
                std::string comand = buf;
                if (comand == "close\n"){
                    _exit(0);
                }
                break;
            }

            }

        }
    }



}

