#include "ardoinocomport.h"
#include <exception>
#include <fstream>
ArduinoComPort::ArduinoComPort(std::string comand, unsigned int speed){
    struct termios serial;


    ComPortDescriptor =open(comand.c_str(),O_RDWR|O_NOCTTY|O_NDELAY);

    if(ComPortDescriptor == -1){
        throw std::runtime_error("Com port not found");
    }

    if(tcgetattr(ComPortDescriptor,&serial)<0){
        close(ComPortDescriptor);
        throw "Com port configuration error";
    }

    serial.c_iflag=0;
    serial.c_oflag=0;
    serial.c_lflag=0;
    serial.c_cflag=0;

    serial.c_cc[VTIME]=0;
    serial.c_cc[VMIN]=0;

    serial.c_cflag=speed|CS8|CREAD|CLOCAL;

    tcsetattr(ComPortDescriptor,TCSANOW,&serial);

}


bool ArduinoComPort::Send(const char *buf, int size){
    if (write(ComPortDescriptor, buf, size) == size){
        return true;
    }
    else
        return false;
}
