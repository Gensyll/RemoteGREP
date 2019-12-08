/*
File: SocketClient.cpp
Author: Matthew Wrobel
Date: 2019-12-05
Brief: Implementation of SocketClient class header
*/
#include "SocketClient.hpp"
#include <string>
#include <sstream>

using namespace std;

SocketClient::SocketClient() : wsaData({}), serverAddress({ 0 }),
hTCPSocket(INVALID_SOCKET), recvBuffer("") {}

SocketClient::~SocketClient() {
	SocketClient::CloseTCPSocket();
}

SocketClient::SocketConnectStatus SocketClient::AttemptForTCPSocketConnection(const char targetAddress[]) {
	//Init Winsock
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		return SocketClient::SocketConnectStatus::WSAStartupFailed;
	}
	//Create socket
	hTCPSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	
	//Set the socket timeout value for receiving data
	/*DWORD iOptVal = 1;
	if (setsockopt(hTCPSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)& iOptVal, sizeof(iOptVal)) == SOCKET_ERROR) {
		cerr << "SocketClient::AttemptForTCPSocketConnection() Socket Option Error: " << WSAGetLastError() << endl;
	}*/

	//Create Server address and attempt server connection
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(RECVPORT);
	inet_pton(AF_INET, targetAddress, &(serverAddress.sin_addr));
	if (connect(hTCPSocket, (SOCKADDR*)& serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {	
		if (WSAGetLastError() == WSAETIMEDOUT) {
			cerr << "Connection attempt timed out." << endl;
		}
		else {
			cerr << "SocketClient::AttemptForTCPSocketConnection() error: " << WSAGetLastError() << endl;
		}		
		SocketClient::CloseTCPSocket();
		return SocketClient::SocketConnectStatus::ConnectFailed;
	}	
	return SocketClient::SocketConnectStatus::Connected;
}

SocketClient::SocketSendStatus SocketClient::ReceiveFromSocket(string &returnVal, int &wsaVal) {
	if (hTCPSocket != INVALID_SOCKET) {
		memset(recvBuffer, 0, sizeof(recvBuffer));
		int bytesReceived = recv(hTCPSocket, recvBuffer, DEFAULT_BUFFER_LENGTH, 0);
		if (bytesReceived == SOCKET_ERROR) {
			returnVal = WSAGetLastError() + "";
			wsaVal = WSAGetLastError();
			return SocketClient::SocketSendStatus::SocketError;
		}
		else {
			stringstream formattedResults;
			string clientResults = recvBuffer;
			while (clientResults.length() > 0) {
				int recSize = stoi(clientResults.substr(0, clientResults.find('|')));
				formattedResults << clientResults.substr(clientResults.find('|') + 1, recSize) << endl;
				clientResults = clientResults.substr(clientResults.find('|') + 1 + recSize);
			}			
			returnVal = formattedResults.str();
			wsaVal = WSAGetLastError();
			return SocketClient::SocketSendStatus::Success;			
		}
	}	
	returnVal = WSAGetLastError() + "";	
	return SocketClient::SocketSendStatus::NoSocket;	
}

SocketClient::SocketSendStatus SocketClient::SendToSocket(string content) {
	if (hTCPSocket != INVALID_SOCKET) {
		stringstream formattedContent;
		formattedContent << (int)strlen(content.c_str()) << "|" << content.c_str();	//Prepend content size for receiving
		int bytesSent = send(hTCPSocket, formattedContent.str().c_str(), (int)strlen(formattedContent.str().c_str()), 0);
		if (bytesSent == SOCKET_ERROR) {			
			cerr << "SendToSocket() error: " << WSAGetLastError() << endl;			
			return SocketClient::SocketSendStatus::SocketError;
		}
		return SocketClient::SocketSendStatus::Success;
	}
	return SocketClient::SocketSendStatus::NoSocket;
}

void SocketClient::CloseTCPSocket() {
	if (hTCPSocket != INVALID_SOCKET) {
		SocketClient::SendToSocket("drop");
		closesocket(hTCPSocket);
		hTCPSocket = INVALID_SOCKET;
		WSACleanup();
	}		
}

bool SocketClient::IsConnected() {
	if (hTCPSocket == INVALID_SOCKET)
		return false;
	return true;
}