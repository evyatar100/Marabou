//
// Created by evyatar on 05/04/2020.
//

#ifndef SOCKETTRANING_TENSORFLOWSOCKET_H
#define SOCKETTRANING_TENSORFLOWSOCKET_H

#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string>

using namespace std;


class SocketExp: public exception
{
private:
    string what_msg;

public:
    SocketExp(string what_msg): what_msg(what_msg){}
    virtual const char* what() const throw()
    {
        return what_msg.c_str();
    }
};

class TensorFlowSocket
{

public:
    TensorFlowSocket();
    ~TensorFlowSocket();
    string runModel(string input);
//    void initPythonServer();

private:

    int sock;
    char buf[4096];
    int port = 43565;

    bool isInitSucsses;
};

#endif //SOCKETTRANING_TENSORFLOWSOCKET_H
