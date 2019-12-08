/*
File: ServerDriver.cpp
Author: Matthew Wrobel
Date: 2019-12-05
Brief: Driver class for execution of remote server instance of UltraGREP Application
*/

#include "SocketServer.hpp"
#include "UltraGrep.hpp"
#include <iostream>
#include <vector>
#include <string>

using namespace std;

vector<string> ParseArguments(string initialFeed) {
	string currArg;
	vector<string> resultArgs, argString;
	for (unsigned i = 0; i < initialFeed.length(); ++i) {
		char c = initialFeed[i];
		if (c == ' ') {
			argString.push_back(currArg);
			currArg.clear();
		}
		else if (c == '\"') {
			++i;
			while (initialFeed[i] != '\"') { currArg.push_back(initialFeed[i]); ++i; }
			argString.push_back(currArg);
			currArg.clear();
		}
		else {
			currArg.push_back(c);
		}
	}
	argString.push_back(currArg);
	for (auto arg : argString) {
		if (strcmp(arg.c_str(), "") != 0) {
			resultArgs.push_back(arg);
		}		
	}
	return resultArgs;
}

int main(int argc, char* argv[]) {
	string targetAddress = "127.0.0.1";
	SocketServer tcpSocket;
	bool waitingOnClients = true;	
	string clientLastInput = "";
	int socketReturnVal = 0;

	try {
		std::cout << "Project 3: Remote UltraGREP - TCP/IP Server (Matthew Wrobel 2019)" << endl;
		
		//Process parameters
		if (argc > 2 || argc < 1) {
			throw exception("Invalid number of parameters. Usage: RemoteGREP [ip-address]");
		}		
		if (argc == 2) { targetAddress = argv[1]; }
		
		//Initial connection
		std::cout << "Establishing TCP/IP connection to " << targetAddress << "..." << endl;
		switch (tcpSocket.BindTCPSocket(targetAddress.c_str())) {
			case SocketServer::SocketBindStatus::WSAStartupFailed:
				cerr << "Windows Socket API Startup failed." << endl;
				return EXIT_FAILURE;
			case SocketServer::SocketBindStatus::BindFailed:
				cerr << "TCP/IP Socket binding failed." << endl;
				return EXIT_FAILURE;
			case SocketServer::SocketBindStatus::ListenFailed:
				cerr << "Enable TCP/IP Socket listening state failed." << endl;
				return EXIT_FAILURE;
			default:
				std::cout << "TCP/IP Socket successfully bound to " << targetAddress << "!" << endl << endl;
				break;
		}

		
		while (waitingOnClients) {			
			socketReturnVal = 0;
			if (tcpSocket.IsClientConnected()) {				
				//Grab socket content and parse accordingly		
				tcpSocket.ReceiveFromClient(clientLastInput);
				switch (tcpSocket.GetWSAResultValue()) {
				case SocketServer::SocketSendStatus::NoSocket:
					cerr << "NoSocket" << socketReturnVal << endl;
					break;
				case SocketServer::SocketSendStatus::SocketError:
					if (socketReturnVal == WSAECONNRESET) {
						tcpSocket.SeverClientConnection();
						std::cout << "Connection to the client has been dropped." << endl;
					}
					break;
				default:
					if (clientLastInput != "") {
						std::cout << "[REMOTE]->" << clientLastInput << endl;
					}
					if (strcmp(clientLastInput.c_str(), "drop") == 0) {
						tcpSocket.SeverClientConnection();
						std::cout << "Connection to the client has been dropped." << endl;
					}
					else if (strcmp(clientLastInput.substr(0, 10).c_str(), "stopserver") == 0) {
						tcpSocket.SendToClient("Server instance is being terminated.");
						tcpSocket.CloseTCPSocket();
						std::cout << "Server instance has been terminated." << endl;
						return EXIT_SUCCESS;
					}
					else if (strcmp(clientLastInput.substr(0, 4).c_str(), "grep") == 0) {
						tcpSocket.SendToClient("starting the Greb");

						vector<string> argList = ParseArguments(clientLastInput);

						
						PerformUltraGrep((int)argList.size(), argList, tcpSocket);
						tcpSocket.SendToClient("finishgrep");
					}
					break;
				}								
			}
			else {
				std::cout << "Awaiting remote connection to TCP/IP Socket..." << endl;
				tcpSocket.WaitForTCPClientConnection();
				std::cout << "Remote connection established." << endl;
			}
		}
	}
	catch (exception ex) {
		cerr << ex.what() << endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}