//
// Created by evyatar on 05/04/2020.
//

#include <iostream>
#include <fstream>
#include "TensorFlowSocket.h"

TensorFlowSocket::TensorFlowSocket()
{
	isInitSucsses = false;
}

void TensorFlowSocket::reInitiateSocket()
{
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1)
	{
		std::cout << "connectRes == -1" << std::endl;
		isInitSucsses = false;
	}

	std::ifstream infile(SERVER_PARAMS_PATH);
	infile >> server_ip >> server_port;
	infile.close();


	std::cout << "Trying to connect to server, with ip " << server_ip << " and port " << server_port
			  << "...\n";

	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(server_port);
	inet_pton(AF_INET, server_ip.c_str(), &hint.sin_addr);

	//	Connect to the server on the socket
	int connectRes = connect(sock, (sockaddr *) &hint, sizeof(hint));
	if (connectRes == -1)
	{
		std::cout << "connectRes == -1" << std::endl;
		isInitSucsses = false;
	} else
	{
		isInitSucsses = true;
	}

	std::cout << "Connected to server successfully, with ip " << server_ip << " and port " << server_port
			  << ".\n";
}

bool TensorFlowSocket::isSocketReady()
{
	return isInitSucsses;
}

string TensorFlowSocket::runModel(string input)
{
	if (!isInitSucsses)
	{
		throw SocketExp("TensorFlowSocket initiation failed");
	}
	std::cout << "running model on input size: " << input.size() << '\n';

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
	for (int i = 0; i < BUF_SIZE; i++)
	{
		buf[i] = 0;
	}
	int bytesReceived = recv(sock, buf, BUF_SIZE, 0);
	if (bytesReceived == -1)
	{
		cout << "There was an error getting response from server\n";
		return nullptr;
	} else
	{
		//		Display response
		return string(buf, bytesReceived);
	}
}

TensorFlowSocket::~TensorFlowSocket()
{
	//	Close the socket
	close(sock);
}
