#pragma once
/*
File: SocketClient.hpp
Author: Matthew Wrobel
Date: 2019-12-05
Brief: Header declaration for client-side socket class facade
*/

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>

#pragma comment (lib, "ws2_32.lib")

unsigned short constexpr RECVPORT = 49153;
//unsigned short constexpr SENDPORT = 49154;
unsigned short constexpr DEFAULT_BUFFER_LENGTH = 1024;

class SocketClient {
public:
	enum SocketConnectStatus { Connected, WSAStartupFailed, ConnectFailed };
	enum SocketSendStatus { Success, SocketError, NoSocket };
private:
	WSAData wsaData;
	SOCKET hTCPSocket;
	sockaddr_in serverAddress;	

	char recvBuffer[DEFAULT_BUFFER_LENGTH];
public:
	SocketClient();
	~SocketClient();

	SocketClient::SocketConnectStatus AttemptForTCPSocketConnection(const char[]);
	SocketClient::SocketSendStatus ReceiveFromSocket(std::string &, int &);
	SocketClient::SocketSendStatus SendToSocket(std::string);
	void CloseTCPSocket();
	bool IsConnected();

};