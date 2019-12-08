#pragma once
/*
File: SocketServer.hpp
Author: Matthew Wrobel
Date: 2019-12-05
Brief: Header declaration for server-side socket class facade
*/

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>

#pragma comment (lib, "ws2_32.lib")

unsigned short constexpr RECVPORT = 49153;
//unsigned short constexpr SENDPORT = 49154;
unsigned short constexpr DEFAULT_BUFFER_LENGTH = 1024;

class SocketServer {
public:
	enum SocketBindStatus { NoError, WSAStartupFailed, BindFailed, ListenFailed, ConfigFailed };
	enum SocketSendStatus { Success, SocketError, NoSocket };
private:	
	WSAData wsaData;
	SOCKET hTCPSocket;
	sockaddr_in serverAddress;
	SOCKET hConnectionToClient;	

	char recvBuffer[DEFAULT_BUFFER_LENGTH];
	int wsaResultVal;
public:		
	SocketServer();
	~SocketServer();
	
	SocketServer::SocketBindStatus BindTCPSocket(const char[]);
	void WaitForTCPClientConnection();
	SocketServer::SocketSendStatus ReceiveFromClient(std::string &);
	SocketServer::SocketSendStatus SendToClient(std::string);
	void CloseTCPSocket();
	void SeverClientConnection();
	bool IsClientConnected();
	int GetWSAResultValue();
	
};