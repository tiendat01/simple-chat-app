//// MultithreadTCPEchoServer.cpp : Defines the entry point for the console application.
////
//
//#include "stdio.h"
//#include "conio.h"
//#include "string.h"
//#include "ws2tcpip.h"
//#include "winsock2.h"
//#include "process.h"
//#include<vector>
//#include<iostream>
//#include<map>
//#include <sstream>
//#define SERVER_PORT 5500
//#define SERVER_ADDR "127.0.0.1"
//#define BUFF_SIZE 2048
//#pragma comment(lib, "ws2_32.lib")
//typedef struct Socket {
//	SOCKET socket;
//	int id;
//}Socket;
//using namespace std;
//vector<Socket>vt;
//
//using namespace std;
//string convertToString(char* a) {
//	string s = a;
//	return s;
//}
//unsigned __stdcall recvThread(void *param) {
//	char buff[BUFF_SIZE];
//	int ret;
//	Socket *s = (Socket*)param;
//	SOCKET connectedSocket = s->socket;
//	int id = s->id;
//	while (1) {
//		ret = recv(connectedSocket, buff, BUFF_SIZE, 0);
//		if (ret == SOCKET_ERROR) {
//			printf("Error %d: Cannot receive data.\n", WSAGetLastError());
//			break;
//		}
//		else if (ret == 0) {
//			printf("Client disconnects.\n");
//			break;
//		}
//		else if (strlen(buff) > 0) {
//			buff[ret] = 0;
//			for (int i = 0; i < vt.size(); i++) {
//				if (id != vt[i].id) {
//					ret = send(vt[i].socket, buff, strlen(buff), 0);
//				}
//			}
//		}
//	}
//	//closesocket(connectedSocket);
//	return 0;
//}
//
//int main(int argc, char* argv[])
//{
//
//	//Step 1: Initiate WinSock
//	WSADATA wsaData;
//	WORD wVersion = MAKEWORD(2, 2);
//	if (WSAStartup(wVersion, &wsaData)) {
//		printf("Winsock 2.2 is not supported\n");
//		return 0;
//	}
//
//	//Step 2: Construct socket	
//	SOCKET listenSock;
//	listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
//
//	//Step 3: Bind address to socket
//	sockaddr_in serverAddr;
//	serverAddr.sin_family = AF_INET;
//	serverAddr.sin_port = htons(SERVER_PORT);
//	inet_pton(AF_INET, SERVER_ADDR, &serverAddr.sin_addr);
//	if (bind(listenSock, (sockaddr *)&serverAddr, sizeof(serverAddr)))
//	{
//		printf("Error %d: Cannot associate a local address with server socket.", WSAGetLastError());
//		return 0;
//	}
//
//	//Step 4: Listen request from client
//	if (listen(listenSock, 10)) {
//		printf("Error %d: Cannot place server socket in state LISTEN.", WSAGetLastError());
//		return 0;
//	}
//
//	printf("Server started!\n");
//
//	//Step 5: Communicate with client
//	SOCKET connSocket;
//	sockaddr_in clientAddr;
//	char clientIP[INET_ADDRSTRLEN];
//	int clientAddrLen = sizeof(clientAddr), clientPort;
//	int i = 1;
//	while (1) {
//		connSocket = accept(listenSock, (sockaddr *)& clientAddr, &clientAddrLen);
//		if (connSocket == SOCKET_ERROR)
//			printf("Error %d: Cannot permit incoming connection.\n", WSAGetLastError());
//		else {
//			inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, sizeof(clientIP));
//			clientPort = ntohs(clientAddr.sin_port);
//			printf("Accept incoming connection from %s:%d\n", clientIP, clientPort);
//			Socket *s = new Socket[1];
//			s->socket = connSocket;
//			s->id = i;
//			i++;
//			vt.push_back(*s);
//			_beginthreadex(0, 0, recvThread, (void *)s, 0, 0); //start thread
//		}
//	}
//
//	closesocket(listenSock);
//
//	WSACleanup();
//
//	return 0;
//}











// SingleIOCPServer.cpp : Defines the entry point for the console application.
//

#include <winsock2.h>
#include <WS2tcpip.h>
#include <windows.h>
#include <stdio.h>
#include <process.h>
#include <iostream>
#include <conio.h>
#include <string>
#include <list>
#include <map>
#include <utility>
#include <fstream>
#include <queue>
#include <cstring>

using namespace std;

#define PORT 5500
#define DATA_BUFSIZE 8192
#define RECEIVE 0
#define SEND 1
#define SERVER_ADDR "127.0.0.1"
#define DELIMITER "\r\n"

#define LOGIN_MSG "LOGIN"

#pragma comment(lib, "Ws2_32.lib")

// Structure definition
typedef struct {
	WSAOVERLAPPED overlapped;
	WSABUF dataBuff;
	CHAR buffer[DATA_BUFSIZE];
	int recvBytes;
	int sentBytes;
	int operation;
} PER_IO_OPERATION_DATA, *LPPER_IO_OPERATION_DATA;

// UserSession
typedef struct {
	SOCKET socket;
	SOCKADDR_IN clientAddr;
	int status; // 0 or 1 for login
	string username;
} PER_HANDLE_DATA, *LPPER_HANDLE_DATA;

typedef struct {
	HANDLE completionPort;
	SOCKET listenSock;
	SOCKADDR_IN serverAddr;
	list<SOCKET> clientSockList;
} SOCKET_IOCP, *LPSOCKET_IOCP;

map<string, pair<string, char>> users;

//// khai bao mang cho nhieu client
//PER_HANDLE_DATA clients[1000];

unsigned __stdcall serverWorkerThread(LPVOID CompletionPortID);


string outputResponseFrom(string request) {
	// LOGIN
	if (request.find("LOGIN") != string::npos) {
		request = request.substr(0, strlen(LOGIN_MSG) + 1);
		return handleLoginRequest(request);
	}
}



int main(int argc, CHAR* argv[])
{
	SOCKET acceptSock;

	SOCKADDR_IN clientAddr;

	//HANDLE completionPort;
	LPSOCKET_IOCP iocp = new SOCKET_IOCP(); // server socket info and IOCP

	SYSTEM_INFO systemInfo;

	LPPER_HANDLE_DATA perHandleData;
	LPPER_IO_OPERATION_DATA perIoData;

	DWORD transferredBytes;
	DWORD flags;

	WSADATA wsaData;

	// Step 0: read file account.txt
	ifstream file(".\\database\\account.txt");
	string line;
	if (!file.is_open()) {
		cerr << "Could not open file " << "\\database\\account.txt" << endl;
		return 1;
	}
	while (getline(file, line)) {
		string delimeter = " ";
		string username = line.substr(0, line.find(delimeter));
		string password = line.substr(line.find(delimeter) + 1, line.rfind(delimeter) - line.find(delimeter) - 1);
		char status = line.substr(line.rfind(delimeter) + 1).at(0);
		cout << username << password << status << endl;
		users.insert({ username, make_pair(password, status) });
	}


	// Step 1: Init Winsock
	if (WSAStartup((2, 2), &wsaData) != 0) {
		printf("WSAStartup() failed with error %d\n", GetLastError());
		return 1;
	}

	// Step 2: Create an I/O completion port
	if ((iocp->completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0))
		== NULL) {
		printf("CreateIoCompletionPort() failed with error %d\n", GetLastError());
		return 1;
	}

	// Step 3: Determine how many processors are on the system
	GetSystemInfo(&systemInfo);

	// Step 4: Create worker threads based on the number of processors available on the
	// system. Create two worker threads for each processor	
	for (int i = 0; i < (int)systemInfo.dwNumberOfProcessors * 2; i++) {
		// Create a server worker thread and pass the completion port to the thread
		if (_beginthreadex(0, 0, serverWorkerThread, (void*)iocp, 0, 0) == 0) {
			printf("Create thread failed with error %d\n", GetLastError());
			return 1;
		}
	}

	// Step 5: Create a listening socket
	if ((iocp->listenSock = WSASocketW(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED))
		== INVALID_SOCKET) {
		printf("WSASocket() failed with error %d\n", WSAGetLastError());
		return 1;
	}

	// bind
	iocp->serverAddr.sin_family = AF_INET;
	iocp->serverAddr.sin_port = htons(PORT);
	inet_pton(AF_INET, SERVER_ADDR, &iocp->serverAddr.sin_addr);
	if (bind(iocp->listenSock, (PSOCKADDR)&iocp->serverAddr, sizeof(iocp->serverAddr)) == SOCKET_ERROR) {
		printf("bind() failed with error %d\n", WSAGetLastError());
		return 1;
	}

	// Prepare socket for listening
	if (listen(iocp->listenSock, 10) == SOCKET_ERROR) {
		printf("listen() failed with error %d\n", WSAGetLastError());
		return 1;
	}

	cout << "Server start!\n";

	while (1) {

		// Step 6: Accept connections
		if ((acceptSock = WSAAccept(iocp->listenSock, (SOCKADDR*)&clientAddr, NULL, NULL, 0)) == SOCKET_ERROR) {
			printf("WSAAccept() failed with error %d\n", WSAGetLastError());
			return 1;
		}

		iocp->clientSockList.push_back(acceptSock);


		// Step 7: Create a socket information structure to associate with the socket
		if ((perHandleData = (LPPER_HANDLE_DATA)GlobalAlloc(GPTR, sizeof(PER_HANDLE_DATA))) == NULL) {
			printf("GlobalAlloc() failed with error %d\n", GetLastError());
			return 1;
		}
		char clientIP[INET_ADDRSTRLEN];
		cout << "Accept connection from IP Address: " + string(inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, sizeof(clientIP)))
			+ " Port: " + to_string(ntohs(clientAddr.sin_port));

		// Step 8: Associate the accepted socket with the original completion port
		printf("Socket number %d got connected...\n", acceptSock);
		perHandleData->socket = acceptSock; // tim 1 cho trong va dua vao mang clients: clients[i]->socket = acceptSock
		memcpy(&(perHandleData->clientAddr), &clientAddr, sizeof(clientAddr));
		if (CreateIoCompletionPort((HANDLE)acceptSock, iocp->completionPort, 
			(DWORD)perHandleData, 0) == NULL) {
			printf("CreateIoCompletionPort() failed with error %d\n", GetLastError());
			return 1;
		}

		// Step 9: Create per I/O socket information structure to associate with the WSARecv call
		if ((perIoData = (LPPER_IO_OPERATION_DATA)GlobalAlloc(GPTR, 
			sizeof(PER_IO_OPERATION_DATA))) == NULL) {
			printf("GlobalAlloc() failed with error %d\n", GetLastError());
			return 1;
		}

		ZeroMemory(&(perIoData->overlapped), sizeof(OVERLAPPED));
		perIoData->sentBytes = 0;
		perIoData->recvBytes = 0;
		perIoData->dataBuff.len = DATA_BUFSIZE;
		perIoData->dataBuff.buf = perIoData->buffer;
		perIoData->operation = RECEIVE;
		flags = 0;

		if (WSARecv(perHandleData->socket, &(perIoData->dataBuff), 1, &transferredBytes, &flags, &(perIoData->overlapped), NULL) == SOCKET_ERROR) {
			if (WSAGetLastError() != ERROR_IO_PENDING) {
				printf("WSARecv() failed with error %d\n", WSAGetLastError());
				return 1;
			}
		}
	}

	// close socket:
	for (list<SOCKET>::iterator it = iocp->clientSockList.begin();
	it != iocp->clientSockList.end(); it++) {
		closesocket(*it);
	}
	iocp->clientSockList.empty();
	CloseHandle(iocp->completionPort);
	
	for (int i = 0; i < (int)systemInfo.dwNumberOfProcessors * 2; i++) {
		PostQueuedCompletionStatus(iocp, 0, NULL, NULL);
	}
	closesocket(iocp->listenSock);
	delete iocp;

	WSACleanup();

	_getch();
	return 0;
}


// voi nhieu clients, trong workerthread phai tim kiem clients[i] vua hoan thanh thao tac vao ra
// loi vao ra => xoa phan tu clients[i] ra khoi mang ==> can dieu do luong
unsigned __stdcall serverWorkerThread(LPVOID iocp)
{
	LPSOCKET_IOCP lpIocp = (LPSOCKET_IOCP)iocp;

	DWORD transferredBytes;
	LPPER_HANDLE_DATA perHandleData;
	LPPER_IO_OPERATION_DATA perIoData;
	DWORD flags;

	string prevBuff;
	queue<string> q;

	while (TRUE) {
		if (GetQueuedCompletionStatus(lpIocp->completionPort, &transferredBytes,
			(LPDWORD)&perHandleData, (LPOVERLAPPED *)&perIoData, INFINITE) == 0) {
			printf("GetQueuedCompletionStatus() failed with error %d\n", GetLastError());
			return 0;
		}
		// Check to see if an error has occurred on the socket and if so
		// then close the socket and cleanup the SOCKET_INFORMATION structure
		// associated with the socket
		if (transferredBytes == 0) {
			printf("Closing socket %d\n", perHandleData->socket);
			if (closesocket(perHandleData->socket) == SOCKET_ERROR) {
				printf("closesocket() failed with error %d\n", WSAGetLastError());
				return 0;
			}
			GlobalFree(perHandleData);
			GlobalFree(perIoData);
			// free client socket
			lpIocp->clientSockList.remove(perHandleData->socket);
			closesocket(perHandleData->socket);
			continue;
		}
		// Check to see if the operation field equals RECEIVE. If this is so, then
		// this means a WSARecv call just completed so update the recvBytes field
		// with the transferredBytes value from the completed WSARecv() call
		if (perIoData->operation == RECEIVE) {
			int found = 0;
			string receivceBuffer = string(perIoData->buffer);
			while ((found = receivceBuffer.find("\r\n")) != string::npos) {
				receivceBuffer = prevBuff + receivceBuffer;
				q.push(receivceBuffer.substr(0, found));
				cout << receivceBuffer.substr(0, found) << endl;
				prevBuff.erase();
				receivceBuffer = receivceBuffer.substr(found + strlen(DELIMITER));
			}
			if (receivceBuffer.length() > 0)
				prevBuff = receivceBuffer;



			// handle request
			while (!q.empty()) {


				string response = outputResponseFrom(q.front());

				LPPER_IO_OPERATION_DATA perIoDataToSend = new PER_IO_OPERATION_DATA();
				ZeroMemory(&(perIoDataToSend->overlapped), sizeof(OVERLAPPED));
				memset(perIoDataToSend, 0, sizeof(PER_IO_OPERATION_DATA));
				memcpy(perIoDataToSend->buffer, response.c_str(), transferredBytes);
				perIoDataToSend->dataBuff.buf = perIoDataToSend->buffer;
				perIoDataToSend->dataBuff.len = transferredBytes;
				perIoDataToSend->operation = SEND;

				// send to one client
				DWORD bytesToSend = 0;
				if (WSASend(perHandleData->socket, &(perIoDataToSend->dataBuff), 1,
					&bytesToSend, 0, &perIoDataToSend->overlapped, NULL) == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
					if (perHandleData != NULL) {
						closesocket(perHandleData->socket);
					}



				//// group chat broadcast
				//list<SOCKET>::iterator it = lpIocp->clientSockList.begin();
				//while (it != lpIocp->clientSockList.end()) {
				//	LPPER_IO_OPERATION_DATA perIoDataToSend = new PER_IO_OPERATION_DATA();
				//	ZeroMemory(&(perIoDataToSend->overlapped), sizeof(OVERLAPPED));
				//	memset(perIoDataToSend, 0, sizeof(PER_IO_OPERATION_DATA));
				//	memcpy(perIoDataToSend->buffer, perIoData->buffer, transferredBytes);
				//	perIoDataToSend->dataBuff.buf = perIoDataToSend->buffer;
				//	perIoDataToSend->dataBuff.len = transferredBytes;
				//	perIoDataToSend->operation = SEND;

				//	DWORD bytesToSend = 0;
				//	if (WSASend(*it, &(perIoDataToSend->dataBuff), 1,
				//		&bytesToSend, 0, &perIoDataToSend->overlapped, NULL) == SOCKET_ERROR
				//		&& WSAGetLastError() != WSA_IO_PENDING) {
				//		if (perHandleData != NULL) {
				//			if (*it != NULL) {
				//				closesocket(*it);
				//				it = lpIocp->clientSockList.erase(it);
				//			}
				//		}
				//		continue;
				//	}

				//	else {
				//		it++;
				//	}
				//} // end while sending to all clients


				q.pop();
			}


			// server continue being at state RECEIVE
			ZeroMemory(&(perIoData->overlapped), sizeof(OVERLAPPED));
			memset(perIoData, 0, sizeof(PER_IO_OPERATION_DATA));
			perIoData->dataBuff.buf = perIoData->buffer;
			perIoData->dataBuff.len = sizeof(perIoData->buffer);
			perIoData->operation = RECEIVE;
			DWORD bytesToReceive = 0, flag = 0;
			WSARecv(perHandleData->socket, &(perIoData->dataBuff), 1, &bytesToReceive,
				&flag, &(perIoData->overlapped), NULL);

			

			continue;
		}
		else {
			// WSASend() success
			if (perIoData != NULL) {
				delete perIoData;
				perIoData = NULL;
			}
			continue;
		}
	} // end while (1)
	return 0;
}