/*
File: ClientDriver.cpp
Author: Matthew Wrobel
Date: 2019-12-05
Brief: Driver class for execution of client instance of UltraGREP Application
*/

#include "SocketClient.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <list>
#include <regex>

using namespace std;

int main(int argc, char* argv[]) {
	string targetAddress = "127.0.0.1";
	SocketClient tcpSocket;
	bool waitingForInput = true;
	string socketLastOutput= "";
	int socketReturnVal = 0;

	try {
		cout << "Project 3: Remote UltraGREP - TCP/IP Client (Matthew Wrobel 2019)" << endl;

		//Process parameters
		if (argc > 2 || argc < 1) {
			throw exception("Invalid number of parameters. Usage: RemoteGREPClient [ip-address]");
		}
		if (argc == 2) { targetAddress = argv[1]; }

		//Initial connection
		cout << "Establishing TCP/IP connection to " << targetAddress << "..." << endl;
		switch (tcpSocket.AttemptForTCPSocketConnection(targetAddress.c_str())) {
		case SocketClient::SocketConnectStatus::WSAStartupFailed:
			cerr << "Windows Socket API Startup failed." << endl;	
			break;
		case SocketClient::SocketConnectStatus::ConnectFailed:
			cerr << "TCP/IP Socket connection attempt failed." << endl;	
			break;
		default:
			cout << "TCP/IP Socket successfully connected to " << targetAddress << "!" << endl << endl;
			break;
		}

		//User input loop
		string userInput;
		while (waitingForInput) {	
			socketLastOutput.clear();			
			userInput.clear();
			cout << "->";
			getline(std::cin, userInput);
			if (strcmp(userInput.substr(0, 7).c_str(), "connect") == 0) {	//connect only happens on the client
				regex connectRegex("(connect [0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3})");
				if (!regex_match(userInput, connectRegex)) {
					cout << "Invalid connect parameters provided. Usage: connect [ip-address]" << endl;
				}
				else {
					if (tcpSocket.IsConnected()) {
						cout << "Drop the existing connection before attempting to establish a new one." << endl;
					}
					else {
						string userProvidedAddress = userInput.substr(8);
						cout << "Establishing TCP/IP connection to " << userProvidedAddress << "..." << endl;
						switch (tcpSocket.AttemptForTCPSocketConnection(userProvidedAddress.c_str())) {
						case SocketClient::SocketConnectStatus::WSAStartupFailed:
							cerr << "Windows Socket API Startup failed." << endl;	
							break;
						case SocketClient::SocketConnectStatus::ConnectFailed:
							cerr << "TCP/IP Socket connection attempt failed." << endl;		
							break;
						default:
							cout << "TCP/IP Socket successfully connected to " << userProvidedAddress << "!" << endl;							
							break;
						}
					}
				}
			}
			else {	// commands sent to be parsed server-side
				if ((strcmp(userInput.c_str(), "drop") == 0 || strcmp(userInput.substr(0, 10).c_str(), "stopserver") == 0 ||
					strcmp(userInput.substr(0, 4).c_str(), "grep") == 0) && tcpSocket.IsConnected()) {
					if (strcmp(userInput.c_str(), "drop") == 0) {
						if (tcpSocket.IsConnected()) {
							tcpSocket.SendToSocket("drop");
							tcpSocket.CloseTCPSocket();
							//cout << "TCP/IP Socket connection has been dropped." << endl;
						}
						else {
							cout << "No existing TCP/IP Socket connection to drop." << endl;
						}
					}
					else if (strcmp(userInput.substr(0, 10).c_str(), "stopserver") == 0) {
						if (tcpSocket.IsConnected()) {							
							tcpSocket.SendToSocket("stopserver");
							tcpSocket.CloseTCPSocket();
							//cout << "TCP/IP Socket connection has been dropped." << endl;
						}
						else {
							cout << "No existing TCP/IP Socket host connection to stop." << endl;
						}
					}
					else if (strcmp(userInput.substr(0, 4).c_str(), "grep") == 0) {						
						tcpSocket.SendToSocket(userInput.c_str());
 						while (strcmp(socketLastOutput.c_str(), "finishgrep") != 0) {
							switch (tcpSocket.ReceiveFromSocket(socketLastOutput)) {
							case SocketClient::SocketSendStatus::NoSocket:
								tcpSocket.CloseTCPSocket();
								break;
							case SocketClient::SocketSendStatus::SocketError:								
								cerr << "WSAError " << socketReturnVal << endl;
								tcpSocket.CloseTCPSocket();
								break;
							default:
								if (socketLastOutput != "") {
									if (strcmp(socketLastOutput.c_str(), "finishgrep") == 0)
										cout << "Grep command complete." << endl;
									else
										cout << socketLastOutput << endl;
								}
								break;
							}
						}
					}							
				}				
				else {
					tcpSocket.SendToSocket(userInput.c_str());
				}				
			}
		}		
	}
	catch (exception ex) {
		cerr << ex.what() << endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}