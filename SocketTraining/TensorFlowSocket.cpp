//
// Created by evyatar on 05/04/2020.
//

#include <iostream>
#include "TensorFlowSocket.h"

TensorFlowSocket::TensorFlowSocket() {

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        throw SocketExp("sock == -1");
    }

    //	Create a hint structure for the server we're connecting with
    string ipAddress = "127.0.0.1";

    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(port);
    inet_pton(AF_INET, ipAddress.c_str(), &hint.sin_addr);

    //	Connect to the server on the socket
    int connectRes = connect(sock, (sockaddr*)&hint, sizeof(hint));
    if (connectRes == -1)
    {
        throw SocketExp("connectRes == -1");
    }

}

string TensorFlowSocket::runModel(string input) {

    //	While loop:
    string userInput = input;

    //		Send to server
    int sendRes = send(sock, userInput.c_str(), userInput.size() + 1, 0);
    if (sendRes == -1)
    {
        cout << "Could not send to server! Whoops!\n";
        return nullptr;
    }

    //		Wait for response
    memset(buf, 0, 4096);
    int bytesReceived = recv(sock, buf, 4096, 0);
    if (bytesReceived == -1)
    {
        cout << "There was an error getting response from server\n";
        return nullptr;
    }
    else
    {
        //		Display response
        return  string(buf, bytesReceived);
    }
}

TensorFlowSocket::~TensorFlowSocket() {
    //	Close the socket
    close(sock);
}


//void TensorFlowSocket::initPythonServer() {
//    system("source ~/PycharmProjects/env/bin/activate");
//
//    system("source ~/PycharmProjects/env/bin/activate");
//
//    std::string command = "python3 ";
//    std::string filename = "../../nn_learner/server.py";
//    std::string args = std::to_string(port);
//
//    command += filename;
//    command += " ";
//    command += args;
//
//    system(command.c_str());
//}
