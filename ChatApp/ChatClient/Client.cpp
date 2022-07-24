#include <stdio.h>
#include <conio.h>
#include<process.h>
#include<iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>
#include <vector>
#include "Header.h"
#include <sstream>
#include<stack>
#pragma comment(lib, "ws2_32.lib")
using namespace std;

#define LOGIN_REQ "LOGIN"
#define SIGNUP_REQ "REGISTER"
#define LOGOUT_REQ "LOGOUT"
#define DELIMITER " "


#define ACCOUNT_LEAVE_GROUP "LEAVE" 
#define LIST_GROUP "LISTGROUP"
#define LIST_MEMBERS_GROUP "LISTMEMBERS" 
#define LOAD_MESSAGE  "LOADMESSAGE"
#define MESSAGE "MESSAGE"

SOCKET clientSock;
char recvBuff[BUFF_SIZE];

CRITICAL_SECTION criticalSection;
string receiveBuffer;
bool isLogin = false;
SCREEN curScreen = LOGIN_SCREEN;
vector<string> onlineUsersList;
vector<string> joinedGroupList;
vector<int> joinedIDGroupList;
int group = -1;
int inbox = -1;
int codescreen = 0;
stack<int>group1;
string listmessage;
string usernameLogin;

void handleMainMenu();
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


vector<string> simple_tokenizer(string s)
{
	stringstream ss(s);
	string word;
	vector<string>sp;
	while (ss >> word) {
		sp.push_back(word);
	}
	return sp;
}

// cut to ClientHeader.h
int getChoice(int maxChoice) {
	int choice = -1;
	do {
		cout << "\nYour choice: ";
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
	usernameLogin = username;
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


//linh

void showListGroupChat() {
	string req = LOAD_MESSAGE + string(DELIMITER) + to_string(group) + ENDING_DELIMITER;
	int ret = Send(clientSock, req, req.length(), 0);
}

void chatGroup() {
	bool back = false;
	string value;
	while (!back) {

		do
		{
			
			//check if any input.
			if (_kbhit()) {
				system("cls");
				showListGroupChat();
				Sleep(1000);
				cout << "Enter: ";
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
				if (value.length() != 0) {
					string req = "MESSAGE" + string(DELIMITER) + to_string(group) + string(DELIMITER) + value + ENDING_DELIMITER;
					int ret = Send(clientSock, req, req.length(), 0);
					// send to server and save to vector
					// ....

					//print
					std::cout << value << std::endl;
				}
			}

			//check if Esc Pressed
		} while (GetAsyncKeyState(VK_ESCAPE) == 0);

	}
}

//linh
void showListJoinedGroup() {
	joinedGroupList.clear();
	joinedIDGroupList.clear();
	// ...
	string req = string(LIST_GROUP) + ENDING_DELIMITER;
	int ret = Send(clientSock, req, req.length(), 0);
}
//linh
void showListMemberGroup() {
	
	string req = string(LIST_MEMBERS_GROUP) + string(DELIMITER) + to_string(group) + ENDING_DELIMITER;
	int ret = Send(clientSock, req, req.length(), 0);
}

void createNewGroup() {

}


void invite() {

}



void reponseInvitation() {

}

//linh
void leaveGroup() {
	string req = ACCOUNT_LEAVE_GROUP + string(DELIMITER) + to_string(group) + ENDING_DELIMITER;
	int ret = Send(clientSock, req, req.length(), 0);
}


//linh
void chatGroup1() {
	int messageLen = 0;
	bool check = true;
	do {	
		system("cls");
		showListGroupChat();
		//Sleep(500);
		//cout << listmessage << endl;
		Sleep(1000);
		//cout << "Enter: ";
		char buff[BUFF_SIZE];
		memset(buff, 0, BUFF_SIZE);
		fflush(stdin);
		gets_s(buff, BUFF_SIZE);
		messageLen = strlen(buff);
		if (strlen(buff)>0 && buff[0] != '1') {
			string req = "MESSAGE" + string(DELIMITER) + to_string(group) + string(DELIMITER) + string(buff) + ENDING_DELIMITER;
			int ret = Send(clientSock, req, req.length(), 0);
		}
		else {
			if (buff[0] == '1') {
				check = false;
			}
		}
	} while (check);
	codescreen = 0;
}

//linh
void chatInGroup() {
	showListJoinedGroup();
	Sleep(200);
	// load history of group chat
	int idGroup;
	bool checkGroup = false;
	int n = 0;
	do {
		if (n != 0) {
			cout << "Group invalid!\n";
		}
		cout << "Enter Group: ";
		cin >> idGroup;
		n++;
		std::vector<int>::iterator it = std::find(joinedIDGroupList.begin(), joinedIDGroupList.end(), idGroup);
		if (it != joinedIDGroupList.end()) {
			checkGroup = true;
		}
	} while (!checkGroup);
	group = idGroup;
	bool check = true;
	while (check) {
		Sleep(200);
		showChatGroupMenu();
		int choice = getChoice(3);
		switch (choice) {
		case 0:
			group = -1;
			check = false;
			break;
		case 1:
			showListMemberGroup();
			break;
		case 2:
			codescreen = 1;
			chatGroup1();
			break;
		case 3:
			leaveGroup();
			Sleep(200);
			group = -1;
			check = false;
			break;
		default:
			break;
		}

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

void handleResponse(string msg) {
	// LOGIN
	//cout << msg;
	if (msg == "100") {
		cout << "\nLogin successfully\n";
		navigateScreen(MAIN_SCREEN);
	}
	else if (msg == "101") {
		cout << "\nUsername not exist\n";
		navigateScreen(LOGIN_SCREEN);
	}
	else if (msg == "102") {
		cout << "\nPassword incorrect\n";
		navigateScreen(LOGIN_SCREEN);

	}
	else if (msg == "103") {
		cout << "\nThis account has been logged in already in this session!\n";
		navigateScreen(LOGIN_SCREEN);

	}
	else if (msg == "104") {
		cout << "\nThis account has been logged in already in other session!\n";
		navigateScreen(LOGIN_SCREEN);

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
	//linh
	else if (msg[0] == '3' && msg[1] == '6' && msg[2] == '0') {
		msg = msg.substr(msg.find(" ") + 1);
		vector<string>splitmsg = simple_tokenizer(msg);
		for (int i = 0; i < splitmsg.size(); i++) {
			string idGroup = splitmsg[i].substr(0, splitmsg[i].find("/"));
			joinedIDGroupList.push_back(atoi(idGroup.c_str()));
		}
		cout << msg <<endl;
	}
	else if (msg[0] == '3' && msg[1] == '7' && msg[2] == '0') {
		msg = msg.substr(msg.find(" ") + 1);
		cout << msg << endl;
	}
	else if (msg.substr(0, 3) == "380") {
		msg = msg.substr(msg.find(" ") + 1);
		//listmessage = msg;
		cout << msg << endl;
	}
	else if (msg.substr(0, 3) == "390") {
		vector<string>response = simple_tokenizer(msg);
		int groupIdmsg = atoi(response[1].c_str());
		if (group == groupIdmsg && response[2] != usernameLogin && codescreen == 1) {
			system("cls");
			showListGroupChat();
		}
		//cout << "send message successfully" << endl;

	}
	else if (msg.substr(0, 3) == "350") {
		cout << "leave group successfully" << endl;
		group = -1;
		codescreen = 0;
		//handleMainMenu();
	}


	else {
		// msg = "999"
		cout << "\nBad request\n";
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
	// back to M�in Menu
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
	while (1) {
		cout << "list group: \n";
		showListJoinedGroup();
		Sleep(500);
		showGroupMenu();
		int choice = getChoice(5);
		switch (choice) {
			// back to M�in Menu
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
			/*		// leave group by groupid
				case 4:
					leaveGroup();
					break;
					// chat in group by groupid*/
		case 4:
			chatInGroup();
			break;
		}
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
		//system("cls");
		
		handleGroupMenu();
		break;
	case CHAT_GROUP_SCREEN:
		chatGroup1();
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
				//std::cout << message; //display a message to user according to code received

				handleResponse(message);

				receiveBuffer = receiveBuffer.substr(found + 2); //cut the part of the received string that has been processed


			}
		}
	}

	return 0;
}





int main(int argc, char* argv[]) {
	group1.push(-1);

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

	/*int timeout = 1000;
	setsockopt(clientSock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(int));*/

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

