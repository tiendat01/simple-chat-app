#include <stdio.h>
#include <conio.h>
#include<process.h>
#include<iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>
#include <vector>
#include "Header.h"

#pragma comment(lib, "ws2_32.lib")
using namespace std;

#define LOGIN_REQ "LOGIN"
#define SIGNUP_REQ "REGISTER"
#define LOGOUT_REQ "LOGOUT"
#define DELIMITER " "

SOCKET clientSock;
char recvBuff[BUFF_SIZE];

CRITICAL_SECTION criticalSection;
string receiveBuffer;
bool isLogin = false;
SCREEN curScreen = LOGIN_SCREEN;
vector<string> onlineUsersList;
vector<string> joinedGroupList;


// cut to ClientHeader.h
/* The recv() wrapper function */
int Receive(SOCKET s, char *buff, int size, int flags) {
	int n;

	n = recv(s, buff, size, flags);
	if (n == SOCKET_ERROR)
		printf("Error %d: Cannot receive data.\n", WSAGetLastError());
	else if (n == 0)
		printf("Client disconnects.\n");
	return n;
}

// cut to ClientHeader.h
/* The send() wrapper function*/
int Send(SOCKET s, string request, int size, int flags) {
	int n;
	const char* buff = request.c_str();
	n = send(s, buff, size, flags);
	if (n == SOCKET_ERROR)
		printf("Error %d: Cannot send data.\n", WSAGetLastError());

	return n;
}


// cut to ClientHeader.h
int getChoice(int maxChoice) {
	int choice = -1;
	do {
		cout << "Your choice: ";
		cin >> choice;
		//cin.ignore();
	} while (choice > maxChoice || choice < 0);
	return choice;
}


void login() {
	string username, password;
	cout << "Username: "; cin >> username;
	cout << "Password: "; cin >> password;

	string req = LOGIN_REQ + string(DELIMITER) + username + " " + password + ENDING_DELIMITER;

	int ret = Send(clientSock, req, req.length(), 0);

}


void signup() {
	string username, password;
	cout << "Username: "; cin >> username;
	cout << "Password: "; cin >> password;

	string req = SIGNUP_REQ + string(DELIMITER) + username + " " + password + ENDING_DELIMITER;

	int ret = Send(clientSock, req, req.length(), 0);
}


void logout() {
	string req = LOGOUT_REQ + string(ENDING_DELIMITER);
	int ret = Send(clientSock, req, req.length(), 0);
}


// send request to get list of online users and show
void showListOnlineUsers() {
	onlineUsersList.clear();
	//...
}


// choose online user to chat with, return username
string chooseChatUser() {
	return NULL;
}


// chat with
void chatWith(string username) {

	//showHistoryInbox();

	bool back = false;
	string value;
	while (!back) {

		do
		{
			//check if any input.
			if (_kbhit()) {
				//probable user started to type, block to read till the user press Enter.
				std::getline(std::cin, value);

				//if success
				if (std::cin.good()) {
					//cout << "(sent)";
				}
				else {
					//firstly, clear error flag 
					std::cin.clear();
					//ignore 
					std::cin.ignore(10000, '\n');

				}

				// send to server and save to vector
				// ....

				//print
				std::cout << value << std::endl;
			}

			//check if Esc Pressed
		} while (GetAsyncKeyState(VK_ESCAPE) == 0);

		// stopConversationDialog()
		int c = -1;
		cout << "0. Cancel" << endl;
		cout << "1. Back" << endl;
		cout << "2. Block" << endl;
		cout << "your choice:";

		cin >> c;
		switch (c) {
		case 0:
			break;
		case 1:
			back = true;
			break;
		case 2:
			break;
		}
	}

}



void showListJoinedGroup() {
	joinedGroupList.clear();
	// ...
}


void createNewGroup() {

}


void invite() {

}



void reponseInvitation() {

}


void leaveGroup() {

}


void chatInGroup() {
	// load history of group chat
}



void handleResponse(string msg) {
	// LOGIN
	if (msg == "100") {
		navigateScreen(MAIN_SCREEN);
	}
	else if (msg == "101") {
		cout << "\nUsername not exist\n";
	}
	else if (msg == "102") {
		cout << "\nPassword incorrect\n";
	}
	else if (msg == "103") {
		cout << "\nThis account has been logged in already in this session!\n";
	}
	else if (msg == "104") {
		cout << "\nThis account has been logged in already in this session!\n";
	}
	
	// SIGN UP
	else if (msg == "110") {
		cout << "\nCreate new account sucessfully\n";
	}
	
	else if (msg == "111") {
		cout << "\nUsername existed\n";
	}
	
	// 
	// else if ...
	
	else {
		// msg = "999"
		cout << "\nBad request\n";
	}
	
}


void handleLoginMenu() {
	int choice = getChoice(2);
	switch (choice) {
	case 0: 
		closesocket(clientSock);
		WSACleanup();
		exit(0);
		break;
	case 1:
		login();
		break;
	case 2:
		signup();
		break;
	default:
		break;
	}
}

void handleMainMenu() {
	int choice = getChoice(3);
	switch (choice) {
	// logout and exit
	case 0:
		logout();
		closesocket(clientSock);
		WSACleanup();
		exit(0);
		break;
	// logout and back
	case 1:
		logout();
		navigateScreen(LOGIN_SCREEN);
		break;
	// Inbox
	case 2:
		navigateScreen(INBOX_SCREEN);
		break;
	// Group chat
	case 3:
		navigateScreen(GROUP_SCREEN);
		break;
	default:
		break;
	}
}


void handleInboxMenu() {

	int choice = getChoice(1);
	switch (choice) {
	// back to Máin Menu
	case 0:
		navigateScreen(MAIN_SCREEN);
		break;
	
	// go to chat box in while loop
	case 1:
		string username = chooseChatUser();
		chatWith(username); // in while loop
		break;
	}
}


void handleGroupMenu() {
	int choice = getChoice(5);
	switch (choice) {
		// back to Máin Menu
	case 0:
		navigateScreen(MAIN_SCREEN);
		break;
		// create new group chat
	case 1:
		createNewGroup();
		break;
		//
	case 2:
		invite();
		break;
		// 
	case 3:
		reponseInvitation();
		break;
		// leave group by groupid
	case 4: 
		leaveGroup();
		break;
		// chat in group by groupid
	case 5:
		chatInGroup();
		break;
	}

}




void navigateScreen(SCREEN screen) {
	switch (screen) {
	case LOGIN_SCREEN:
		showLoginMenu();
		handleLoginMenu();
		break;
	case MAIN_SCREEN:
		showMainMenu();
		handleMainMenu();
		break;
	case INBOX_SCREEN:
		showListOnlineUsers();
		showInboxMenu();
		handleInboxMenu();
		break;
	case GROUP_SCREEN:
		showListJoinedGroup();
		showGroupMenu();
		handleGroupMenu();
		break;
	default:
		break;

	}
}






unsigned __stdcall recvThread(void *param) {
	char buff[BUFF_SIZE];
	int ret;
	SOCKET connectedSocket = (SOCKET)param;
	while (1) {
		ret = recv(connectedSocket, buff, BUFF_SIZE, 0);
		if (ret == SOCKET_ERROR) {
			printf("Error %d: Cannot receive data.\n", WSAGetLastError());
			break;
		}
		else if (ret == 0) {
			printf("Client disconnects.\n");
			break;
		}
		else if (strlen(buff) > 0) {
			buff[ret] = 0;

			// xu ly truyen dong
			// buff = "...\r\n" hoac "..." hoac "...\r\n..."
			receiveBuffer = string(buff);

			while (receiveBuffer.find("\r\n") != string::npos) {
				size_t found = receiveBuffer.find("\r\n");
				string message = receiveBuffer.substr(0, found); //message contains code received from server
				std::cout << message; //display a message to user according to code received

				handleResponse(message);

				receiveBuffer = receiveBuffer.substr(found + 2); //cut the part of the received string that has been processed
			}
		}
	}

	return 0;
}





int main(int argc, char* argv[]) {

	InitializeCriticalSection(&criticalSection);

	// Validate the input
	if (argc < 3) {
		printf("\nRequire hostname and port in commandline parameter.");
		return 1; // error
	}
	if (!isNumeric(argv[2])) {
		printf("Invalid address port.\n");
		return 1; // error
	}

	// STEP 1: Initiate Winsock
	WSADATA wsaData;
	WORD wVersion = MAKEWORD(2, 2);
	if (WSAStartup(wVersion, &wsaData) == SOCKET_ERROR) {
		printf("Error #%d: Winsock 2.2 not supported.\n", WSAGetLastError());
		return 1; // error
	}

	// STEP 2: Construct socket
	clientSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (clientSock == INVALID_SOCKET) {
		printf("Error #%d: Cannot create server socket.\n", WSAGetLastError());
		return 0;
	}

	int timeout = 1000;
	setsockopt(clientSock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(int));

	printf("Client started!\n");

	// STEP 3: Specify server address
	char serverIP[INET_ADDRSTRLEN];
	u_short serverPort;
	sockaddr_in serverAddr;

	serverPort = (u_short)strtoul(argv[2], NULL, 0);
	strcpy_s(serverIP, argv[1]);

	// specify server socket connection info
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(serverPort);
	inet_pton(AF_INET, serverIP, &serverAddr.sin_addr);

	// STEP 4: request to connect server
	if (connect(clientSock, (sockaddr*)&serverAddr, sizeof(serverAddr))) {
		printf("Error #%d: Cannot connect server.\n", WSAGetLastError());
		return 0;
	}

	printf("Connected to server.\n");


	// STEP 5: Communicate with server
	HANDLE myHandle[2];

	myHandle[0] = (HANDLE)_beginthreadex(0, 0, recvThread, (void*)clientSock, 0, 0); //start thread
	myHandle[1] = (HANDLE)_beginthreadex(0, 0, recvThread, (void*)clientSock, 0, 0); //start thread

	navigateScreen(LOGIN_SCREEN);

	WaitForMultipleObjects(2, myHandle, TRUE, INFINITE);

	DeleteCriticalSection(&criticalSection);

	

	// STEP 6: Close socket
	if (closesocket(clientSock) == SOCKET_ERROR) {
		printf("Error #%d: Cannot close listenSocket.\n", WSAGetLastError());
		return 0;
	}

	// STEP 7: Terminate Winsock
	if (WSACleanup() == SOCKET_ERROR) {
		printf("Error #%d: Cannot terminate Winsock.\n", WSAGetLastError());
		return 0;
	}

	return 0;
}

