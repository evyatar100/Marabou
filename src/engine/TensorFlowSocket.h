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

#define BUF_SIZE 32768 // we usually need around 13000 chars
#define SERVER_PARAMS_PATH "/cs/labs/guykatz/evyatar100/Marabou/split_analyzer_NN/server_params.txt"

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

    void reInitiateSocket();
    bool isSocketReady();
    string runModel(string input);
//    void initPythonServer();

private:

    int sock;
	bool isInitSucsses;
	char buf[BUF_SIZE];
	int server_port;
	string server_ip;
};

#endif //SOCKETTRANING_TENSORFLOWSOCKET_H
