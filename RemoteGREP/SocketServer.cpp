/*
File: SocketServer.cpp
Author: Matthew Wrobel
Date: 2019-12-05
Brief: Implementation of SocketServer class header
*/
#include "SocketServer.hpp"
#include <string>
#include <sstream>

using namespace std;

SocketServer::SocketServer() : wsaData({}), serverAddress({ 0 }),
hTCPSocket(INVALID_SOCKET), hConnectionToClient(INVALID_SOCKET),recvBuffer("") {}

SocketServer::~SocketServer() {
	SocketServer::CloseTCPSocket();
}

SocketServer::SocketBindStatus SocketServer::BindTCPSocket(const char targetAddress[]) {
	//Init Winsock	
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) == SOCKET_ERROR) {
		return SocketServer::SocketBindStatus::WSAStartupFailed;
	}		
	hTCPSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	//Create Server address and bind to socket
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(RECVPORT);
	inet_pton(AF_INET, targetAddress, &(serverAddress.sin_addr));
	if (bind(hTCPSocket, (SOCKADDR*)& serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
		cerr << "bind() error: " << WSAGetLastError() << endl;
		SocketServer::CloseTCPSocket();
		return SocketServer::SocketBindStatus::BindFailed;
	}	

	//Set socket to listening state
	if (listen(hTCPSocket, 1) == SOCKET_ERROR) {
		SocketServer::CloseTCPSocket();
		return SocketServer::SocketBindStatus::ListenFailed;
	}

	return SocketServer::SocketBindStatus::NoError;
}

void SocketServer::WaitForTCPClientConnection() {
	while (hConnectionToClient == INVALID_SOCKET) {
		hConnectionToClient = accept(hTCPSocket, NULL, NULL);
	}
}

SocketServer::SocketSendStatus SocketServer::ReceiveFromClient(string& returnVal, int& wsaVal) {
	if (hConnectionToClient != INVALID_SOCKET) {
		memset(recvBuffer, 0, sizeof(recvBuffer));
		int bytesReceived = recv(hConnectionToClient, recvBuffer, DEFAULT_BUFFER_LENGTH, 0);
		if (bytesReceived == SOCKET_ERROR) {
			returnVal = WSAGetLastError() + "";
			wsaVal = WSAGetLastError();
			return SocketServer::SocketSendStatus::SocketError;
		}
		else {
			string clientResults = recvBuffer;
			int recSize = stoi(clientResults.substr(0, clientResults.find('|')));			
			returnVal = clientResults.substr(clientResults.find('|') + 1, recSize);
			wsaVal = WSAGetLastError();
			return SocketServer::SocketSendStatus::Success;			
		}
	}
	returnVal = WSAGetLastError() + "";
	wsaVal = WSAGetLastError();
	return SocketServer::SocketSendStatus::NoSocket;
}

SocketServer::SocketSendStatus SocketServer::SendToClient(string content, int& wsaVal) {
	if (hConnectionToClient != INVALID_SOCKET) {
		stringstream formattedContent;
		formattedContent << (int)strlen(content.c_str()) << "|" << content.c_str();	//Prepend content size for receiving
		int bytesSent = send(hConnectionToClient, formattedContent.str().c_str(), (int)strlen(formattedContent.str().c_str()), 0);
		if (bytesSent == SOCKET_ERROR) {
			wsaVal = WSAGetLastError();
			return SocketServer::SocketSendStatus::SocketError;
		}
		wsaVal = WSAGetLastError();
		return SocketServer::SocketSendStatus::Success;
	}	
	wsaVal = WSAGetLastError();
	return SocketServer::SocketSendStatus::NoSocket;
}

void SocketServer::CloseTCPSocket() {
	if (hTCPSocket != INVALID_SOCKET) {
		SocketServer::SeverClientConnection();
		closesocket(hTCPSocket);
		hTCPSocket = INVALID_SOCKET;
		WSACleanup();
	}	
}

void SocketServer::SeverClientConnection() {
	if (hConnectionToClient != INVALID_SOCKET) {
		closesocket(hConnectionToClient);
		hConnectionToClient = INVALID_SOCKET;
	}
}

bool SocketServer::IsClientConnected() {
	if (hConnectionToClient == INVALID_SOCKET)
		return false;
	return true;
}






