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
#define ENDING_DELIMITER "\r\n"

// request message code 
#define LOGIN_REQ "LOGIN"
#define SIGNUP_REQ "REGISTER"
#define LOGOUT_REQ "LOGOUT"

#define CREATE_GROUP_REQ "CREATEGROUP"
#define INVITE_REQ "INVITE"
#define LIST_JOINED_GROUP "LISTGROUP"


// response code
#define LOGIN_SUCCESS "100"
#define ACCOUNT_NOT_FOUND "101"
#define INCORRECT_PASSWORD "102"
#define ACCOUNT_LOGGED_IN "103"
#define ACCOUNT_OTHER_SESSION "104"

#define SIGNUP_SUCCESS "110"
#define ACCOUNT_EXISTED "111"

#define LOGOUT_SUCESS "120"

#define CREATE_GROUP_SUCCESS "300"
#define FOBBIDEN_TOKEN "301"

#define LIST_GROUP_SUCCESS "360"



#define BAD_REQUEST "999"
#define ACCOUNT_NOT_LOGGED_IN "201"

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


map<string, tuple<string, SOCKET, int>> users;
map<int, string> groups;
//map<string, map<int, string>> groupsPerUsername;

int autoIncrementGroupId = 1; // groupId start from 1 and auto increment


//// khai bao mang cho nhieu client
//PER_HANDLE_DATA clients[1000];

unsigned __stdcall serverWorkerThread(LPVOID CompletionPortID);

void readAccountDb() {
	ifstream file(".\\database\\account.txt");
	string line;
	if (!file.is_open()) {
		cerr << "Could not open file " << "\\database\\account.txt" << endl;
		exit(1);
	}
	users.clear();
	while (getline(file, line)) {
		string delimeter = " ";
		string username = line.substr(0, line.find(delimeter));
		string password = line.substr(line.find(delimeter) + 1);
		cout << username << " " << password << endl;
		users.insert({ username, make_tuple(password, 0, 0) });
	}
	file.close();
}

void readGroupDb() {
	ifstream file(".\\database\\group.txt");
	string line;
	if (!file.is_open()) {
		cerr << "Could not open file " << ".\\database\\group.txt" << endl;
		exit(1);
	}
	groups.clear();
	while (getline(file, line)) {
		string groupIdStr = line.substr(0, line.find(" "));
		string groupDisplayName = line.substr(line.find(" ") + 1);
		int groupId = atoi(groupIdStr.c_str());
		groups.insert({ groupId, groupDisplayName });
	}
	file.close();
}


string handleLoginRequest(string content, LPPER_HANDLE_DATA client) {
	string username = content.substr(0, content.find(" "));
	string password = content.substr(content.find(" ") + 1);

	if (username.length() <= 0)
		return string(BAD_REQUEST);

	if (users.find(username) == users.end())
		return string(ACCOUNT_NOT_FOUND);

	auto info = users.at(username); // info is a tuple

	if (get<0>(info) != password)
		return string(INCORRECT_PASSWORD);

	// already logged in
	if (get<2>(info) == 1) {
		if (client->socket != get<1>(info))
			return string(ACCOUNT_OTHER_SESSION);
		return string(ACCOUNT_LOGGED_IN);
	}

	// login successfully
	client->status = 1;
	client->username = username;
	cout << client->username << endl;

	map<string, tuple<string, SOCKET, int>>::iterator it = users.find(username);

	if (it != users.end()) {
		it->second = make_tuple(password, client->socket, client->status);
	}

	return string(LOGIN_SUCCESS);

}


string handleSignupRequest(string content) {
	string username = content.substr(0, content.find(" "));
	string password = content.substr(content.find(" ") + 1);

	if (username.length() <= 0)
		return string(BAD_REQUEST);

	if (users.find(username) != users.end()) {
		return string(ACCOUNT_EXISTED);
	}
	else {
		// update to database
		ofstream file(".\\database\\account.txt", std::ios_base::app);
		if (!file.is_open()) {
			cerr << "Could not open file " << "\\database\\account.txt" << endl;
			exit(1);
		}
		file << username << " " << password << endl;
		readAccountDb(); // reload data

		file.close();

		return string(SIGNUP_SUCCESS);
	}
}

string handleLogoutRequest(LPPER_HANDLE_DATA client) {

	if (users.find(client->username) != users.end()) {
		auto tuple1 = users.at(client->username);
		if (get<2>(tuple1) == 0)
			return string(ACCOUNT_NOT_LOGGED_IN);
		else {

			map<string, tuple<string, SOCKET, int>>::iterator it = users.find(client->username);

			if (it != users.end()) {
				it->second = make_tuple(get<0>(tuple1), client->socket, 0);
			}
			client->status = 0;
			client->username = "";
			
			return string(LOGOUT_SUCESS);

		}
	}
	return string(BAD_REQUEST);
}

string handleCreateGroupRequest(string request, LPPER_HANDLE_DATA client) {
	
	if (request.find("/") != string::npos)
		return string(FOBBIDEN_TOKEN);
	if (client->status == 0)
		return string(ACCOUNT_NOT_LOGGED_IN);

	int groupId = autoIncrementGroupId++; // from 0
	string groupName = request;

	ofstream file;
	string filename = ".\\database\\group.txt";
	file.open(filename, std::ios_base::app | std::ios_base::out);
	file << groupId << " " << groupName << endl;
	file.close();

	// update map
	readGroupDb();

	filename = ".\\database\\group-members\\group" + to_string(groupId) + ".txt";
	file.open(filename, std::ios_base::app | std::ios_base::out);
	file << client->username << endl;
	file.close();

	filename = ".\\database\\joined-group-by-username\\" + client->username + ".txt";
	file.open(filename, std::ios_base::app | std::ios_base::out);
	file << to_string(groupId) << endl;
	file.close();

	return string(CREATE_GROUP_SUCCESS) + " " + groupName;

}

string handleInviteRequest(string content, LPPER_HANDLE_DATA client) {
	if (client->status == 0)
		return string(ACCOUNT_NOT_LOGGED_IN);
	string destUsername = content.substr(0, content.find(" "));
	int groupId = atoi(content.substr(content.find(" ") + 1).c_str());

	cout << destUsername << " " << groupId;

	// check exception here ...
	if (users.find(destUsername) == users.end())
		return "101";

	if (groups.find(groupId) == groups.end())
		return "312";

	string path = ".\\database\\group-members\\group" + to_string(groupId) + ".txt";
	ifstream file(path);
	if (!file.is_open()) {
		cerr << "Could not open file " << path << endl;
		exit(1);
	}

	string line;
	bool isInGroup = false;
	while (getline(file, line)) {
		if (line.find(destUsername) != string::npos)
			return "313";
		if (line.find(client->username) != string::npos)
			isInGroup = true;
	}

	if (!isInGroup)
		return "341";

}


string handleListGroupRequest(LPPER_HANDLE_DATA client) {
	string path = ".\\database\\joined-group-by-username\\" + client->username + ".txt";
	ifstream file(path);
	string line;
	if (!file.is_open()) {
		cerr << "Could not open file " << path << endl;
		return string(LIST_GROUP_SUCCESS) + "/";
	}
	vector<pair<int, string>> joinedGroupPerUsername;
	string responseBody = string(LIST_GROUP_SUCCESS) + " ";

	while (getline(file, line)) {
		int groupId = atoi(line.c_str());
		if (groups.find(groupId) == groups.end())
			continue;
		responseBody += (line + " " + groups.at(groupId) + "/");

	}

	return responseBody;
}



string outputResponseFrom(string request, LPPER_HANDLE_DATA client) {
	// LOGIN
	if (request.find(LOGIN_REQ) != string::npos && request.find(LOGIN_REQ) == 0 
		&& request.find(" ") == strlen(LOGIN_REQ)) {
		request = request.substr(strlen(LOGIN_REQ) + 1);
		return handleLoginRequest(request, client);
	}

	// SIGNUP
	if (request.find(SIGNUP_REQ) != string::npos && request.find(SIGNUP_REQ) == 0
		&& request.find(" ") == strlen(SIGNUP_REQ)) {
		request = request.substr(strlen(SIGNUP_REQ) + 1);
		return handleSignupRequest(request);
	}

	// LOGOUT
	if (request.find(LOGOUT_REQ) != string::npos && request.find(LOGOUT_REQ) == 0) {
		return handleLogoutRequest(client);
	}


	// CREATE NEW GROUP: CREATEGROUP ... (if not => bad request)
	if (request.find(CREATE_GROUP_REQ) != string::npos && request.find(CREATE_GROUP_REQ) == 0
		&& request.find(" ") == strlen(CREATE_GROUP_REQ)) {
		request = request.substr(strlen(CREATE_GROUP_REQ) + 1);
		return handleCreateGroupRequest(request, client);
	}

	// INVITE TO NEW GROUP: INVITE username groupId (if not => bad request)
	if (request.find(INVITE_REQ) != string::npos && request.find(INVITE_REQ) == 0
		&& request.find(" ") == strlen(INVITE_REQ) && request.substr(request.find(" ") + 1).find(" ") != string::npos) {
		request = request.substr(strlen(INVITE_REQ) + 1);
		return handleInviteRequest(request, client);
	}

	// LIST JOINED GROUP of client
	if (request.find(LIST_JOINED_GROUP) != string::npos && request.find(LIST_JOINED_GROUP) == 0) {
		return handleListGroupRequest(client);
	}


	return string(BAD_REQUEST);
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
	readAccountDb();
	readGroupDb();


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
		printf("\nSocket number %d got connected...\n", acceptSock);
		perHandleData->socket = acceptSock; // tim 1 cho trong va dua vao mang clients: clients[i]->socket = acceptSock
		perHandleData->status = 0;
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
	queue<string> requestQueue;

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
			receivceBuffer = prevBuff + receivceBuffer;
			prevBuff.clear();
			// stream handle
			while ((found = receivceBuffer.find("\r\n")) != string::npos) {
				if (receivceBuffer.find("\r\n") > 0) {
					requestQueue.push(receivceBuffer.substr(0, found));
					//cout << "request from client: " << receivceBuffer.substr(0, found) << endl;
					receivceBuffer = receivceBuffer.substr(found + strlen(ENDING_DELIMITER));
				}
			}
			if (receivceBuffer.length() > 0)
				prevBuff = receivceBuffer;



			// handle request
			while (!requestQueue.empty()) {

				cout << "handle request from client: " << requestQueue.front() << endl;


				// process
				string response = outputResponseFrom(requestQueue.front(), perHandleData);
				
				response += ENDING_DELIMITER;

				// send response to one client
				LPPER_IO_OPERATION_DATA perIoDataToSend = new PER_IO_OPERATION_DATA();
				ZeroMemory(&(perIoDataToSend->overlapped), sizeof(OVERLAPPED));
				memset(perIoDataToSend, 0, sizeof(PER_IO_OPERATION_DATA));
				memcpy(perIoDataToSend->buffer, response.c_str(), response.length());
				perIoDataToSend->dataBuff.buf = perIoDataToSend->buffer;
				perIoDataToSend->dataBuff.len = response.length();
				perIoDataToSend->operation = SEND;

				DWORD bytesToSend = 0;
				if (WSASend(perHandleData->socket, &(perIoDataToSend->dataBuff), 1,
					&bytesToSend, 0, &perIoDataToSend->overlapped, NULL) == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
					if (perHandleData != NULL) {
						closesocket(perHandleData->socket);
						lpIocp->clientSockList.remove(perHandleData->socket);
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


				requestQueue.pop();
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