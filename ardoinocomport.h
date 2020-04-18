#pragma once

#include <stdio.h>
#include <termios.h>
#include <string.h>
#include <fcntl.h>
#include <iostream>
#include <string>
#include <unistd.h>

class ArduinoComPort
{
public:
        ArduinoComPort(std::string comand, unsigned int spee = B9600);
        bool Send(const char* buf, int size);
private:
        int ComPortDescriptor;
};

