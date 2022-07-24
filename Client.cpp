#include <stdio.h>
#include <conio.h>
#include<process.h>
#include<iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>
#include <vector>
#include <map>
#include "Header.h"

#pragma comment(lib, "ws2_32.lib")

using namespace std;

#define LOGIN_REQ "LOGIN"
#define SIGNUP_REQ "REGISTER"
#define LOGOUT_REQ "LOGOUT"
#define CREATE_GROUP_REQ "CREATEGROUP"
#define INVITE_REQ "INVITE"
#define LIST_JOINED_GROUP "LISTGROUP"
#define DELIMITER " "

SOCKET clientSock;
char recvBuff[BUFF_SIZE];

CRITICAL_SECTION criticalSection;
string receiveBuffer;
SCREEN curScreen = LOGIN_SCREEN;

map<int, string> joinedGroupList;
int groupId = -1;
string userChatWith; //Giang
vector<string> onlineUsersList;
vector<string> conversationMessages;


// cut to ClientHeader.h
/* The recv() wrapper function */
int Receive(SOCKET s, char* buff, int size, int flags) {
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

void sendProcessing(string header) {
	string req = header + ENDING_DELIMITER;
	int ret = Send(clientSock, req, req.length(), 0);
}

void sendProcessing(string header, string s1) {
	string req = header + " " + s1 + ENDING_DELIMITER;
	int ret = Send(clientSock, req, req.length(), 0);
}

void sendProcessing(string header, string s1, string s2) {
	string req = header + " " + s1 + " " + s2 + ENDING_DELIMITER;
	int ret = Send(clientSock, req, req.length(), 0);
}



// cut to ClientHeader.h
int getChoice(int maxChoice) {
	string choice;
	do {
		cout << "Your choice: ";
		cin >> choice;
		//cin.ignore();
	} while (choice[0] > 48 + maxChoice || choice[0] < 48 || choice.length() != 1);
	return choice[0] - 48;
}

// SEND REQUEST

void login() {
	string username, password;
	cout << "Username: "; cin >> username; cin.ignore(); // ignore '\n'
	cout << "Password: "; getline(cin, password);

	string req = LOGIN_REQ + string(DELIMITER) + username + " " + password + ENDING_DELIMITER;

	int ret = Send(clientSock, req, req.length(), 0);

}

void signup() {
	string username, password;
	cout << "Register new account: " << endl;
	cout << "Username: "; cin >> username; cin.ignore(); // ignore '\n'
	cout << "Password: "; getline(cin, password);

	string req = SIGNUP_REQ + string(DELIMITER) + username + " " + password + ENDING_DELIMITER;

	int ret = Send(clientSock, req, req.length(), 0);
}


void logout() {
	string req = LOGOUT_REQ + string(ENDING_DELIMITER);
	int ret = Send(clientSock, req, req.length(), 0);
}


// send request to get list of online users and show
void loadOnlineUsers() {
	sendProcessing("LISTONLINE");
}

void inboxAnUser() {
	cout << "Enter username you want to chat with: ";
	cin >> userChatWith;
	sendProcessing("CHECK", userChatWith);
}

void sendInboxMessage() {
	cout << "Enter message: ";
	string message; cin >> message;
	sendProcessing("SEND", userChatWith, message);
}

void showInboxMessages() {
	sendProcessing("LOADCONVERSATION", userChatWith);
}

void blockUser() {
	sendProcessing("BLOCK", userChatWith);
}

void unblockUser() {
	sendProcessing("UNBLOCK", userChatWith);
}



// send request to get list of online users and show
void showListOnlineUsers() {
	cout << "User Online:\n";
	for (int i = 0; i < onlineUsersList.size(); i++) cout << (i + 1) << " " << onlineUsersList[i] << "\n";
}


// send request to reload JoinedGroupList
void showListJoinedGroup() {

	string req = LIST_JOINED_GROUP + string(ENDING_DELIMITER);
	int ret = Send(clientSock, req, req.length(), 0);

}
//void showListJoinedGroup() {
//
//
//
//	cout << "choose group by id ...";
//
//}


void createNewGroup() {
	string groupDisplayName;
	cout << "Enter your new Group name (forbidden character '/'): ";
	cin.ignore(); getline(cin, groupDisplayName);

	string req = CREATE_GROUP_REQ + string(" ") + groupDisplayName + string(ENDING_DELIMITER);
	int ret = Send(clientSock, req, req.length(), 0);

}


void invite() {
	string destUsername;
	int groupId;
	cout << "Enter your joined group id: "; cin >> groupId;
	cout << "Enter username of person you want to invite: "; cin >> destUsername;

	string req = INVITE_REQ + string(" ") + destUsername + " " + to_string(groupId) + string(ENDING_DELIMITER);
	int ret = Send(clientSock, req, req.length(), 0);

}



void reponseInvitation() {
	// accept or deny
}


void leaveGroup() {

}


void chatInGroup() {
	// load history of group chat
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
		break;
		// Go to chat menu
	case 2:
		loadOnlineUsers();
		break;
		// Group chat
	case 3:
		navigateScreen(GROUP_SCREEN);
		break;
	default:
		break;
	}
}



void handleChatMenu() {

	int choice = getChoice(1);
	switch (choice) {
		// back to Main Menue
	case 0:
		navigateScreen(MAIN_SCREEN);
		break;

		//send inbox request to server
	case 1:
		inboxAnUser();
		break;
	}
}

void handleInboxMenu() {
	int choice = getChoice(4);
	switch (choice) {

	case 0: //Back to chat menu
		loadOnlineUsers();
		break;
	case 1: //Send message
		sendInboxMessage();
		break;
	case 2: //Show history messages
		showInboxMessages();
		break;
	case 3: //Block this user
		blockUser();
		break;
	case 4: //Unblock this user
		unblockUser();
		break;
	}
}

void handleGroupMenu() {
	int choice = getChoice(4);
	switch (choice) {
		// back to Main Menu
	case 0:
		curScreen = MAIN_SCREEN;
		navigateScreen(MAIN_SCREEN);
		break;
		// create new group chat
	case 1:
		createNewGroup();
		break;
		// invite new person to your group
	case 2:
		invite();
		break;
		// Accept/Deny invitation to a group
	case 3:
		reponseInvitation();
		break;
		// chat in group by groupid
	case 4:
		showListJoinedGroup();
		break;
	}

}



// @Not Complete
void handleChatGroupMenu() {
	int choice = getChoice(3);
	switch (choice)
	{
		// back to GROUP SCREEN
	case 0:
		curScreen = GROUP_SCREEN;
		navigateScreen(GROUP_SCREEN);
		break;
		// list online members in group
	case 1:
		break;
		// chat in group
	case 2:
		break;
		// leave group
	case 3:
		break;
	default:
		break;
	}
}


void handleConversation() {
	printf("\n\n-------------- %s --------------\n", userChatWith.c_str());
	printf("Press '1' to leave\n\n");
	for (string message : conversationMessages) {
		cout << message << "\n";
	}
	string leave; cin >> leave;
	if (leave == "1") {
		navigateScreen(INBOX_SCREEN);
	}
}



/// HANDLE RESPONSE FROM CLIENT

// need refactory and cut to Header.h
void handleResponse(string msg) {

	// LOGIN
	if (msg == "100") {
		cout << "\nLogin successfully\n";
		curScreen = MAIN_SCREEN;
		navigateScreen(MAIN_SCREEN);
	}
	// LOGIN + INVITE
	else if (msg == "101") {
		cout << "\nUsername not exist\n";
		navigateScreen(curScreen);
	}
	// LOGIN
	else if (msg == "102") {
		cout << "\nPassword incorrect\n";
		curScreen = LOGIN_SCREEN;
		navigateScreen(LOGIN_SCREEN);

	}
	// LOGIN
	else if (msg == "103") {
		cout << "\nAn account has been logged in already in this session!\n";
		curScreen = LOGIN_SCREEN;
		navigateScreen(LOGIN_SCREEN);

	}
	// LOGIN
	else if (msg == "104") {
		cout << "\nThis account has been logged in already in other session!\n";
		curScreen = LOGIN_SCREEN;
		navigateScreen(LOGIN_SCREEN);

	}

	// SIGN UP
	else if (msg == "110") {
		cout << "\nCreate new account sucessfully\n";
		curScreen = LOGIN_SCREEN;
		navigateScreen(LOGIN_SCREEN);
	}
	// SIGN UP
	else if (msg == "111") {
		cout << "\nUsername existed\n";
		cout << "Do you want to continue to sign up ? (0: Back / 1: Continue) ";
		int choice = getChoice(1);
		if (choice == 1)
			signup();
		else {
			curScreen = LOGIN_SCREEN;
			navigateScreen(LOGIN_SCREEN);
		}
	}

	// LOG OUT
	else if (msg == "120") {
		cout << "\nLog out successfully\n";
		curScreen = LOGIN_SCREEN;
		navigateScreen(LOGIN_SCREEN);
	}

	//Chat 1-1
	else if (msg.substr(0, 3) == "200") { //Return list onine users 
		string online = msg.substr(4);
		onlineUsersList.clear();
		while (online.find(" ") != string::npos) {
			size_t found = online.find(" ");
			onlineUsersList.push_back(online.substr(0, found));
			online = online.substr(found + 1);
		}
		onlineUsersList.push_back(online);
		navigateScreen(CHAT_SCREEN);
	}

	else if (msg == "210") {
		navigateScreen(INBOX_SCREEN);
	}

	else if (msg == "211") {
		cout << "\nThis username doesn't exist\n\n";
		loadOnlineUsers();
	}

	else if (msg.substr(0, 3) == "220") {
		cout << "\n\n";
		showInboxMessages();
	}

	else if (msg.substr(0, 3) == "230") {
		string allMessages = msg.substr(4);
		conversationMessages.clear();
		while (allMessages.find("\n") != string::npos) {
			size_t found = allMessages.find("\n");
			conversationMessages.push_back(allMessages.substr(0, found));
			allMessages = allMessages.substr(found + 1);
		}
		onlineUsersList.push_back(allMessages);
		navigateScreen(CONVERSATION_SCREEN);
	}

	else if (msg.substr(0, 3) == "240") {
		string content = msg.substr(4);
		string username = content.substr(0, content.find(" "));
		string newMessage = content.substr(content.find(" ") + 1);
		if (curScreen == CONVERSATION_SCREEN && userChatWith == username) {
			cout << newMessage << "\n";
		}
	}

	else if (msg == "250") {
		cout << "\nBlock successfully\n\n";
		navigateScreen(INBOX_SCREEN);
	}

	else if (msg == "221") {
		cout << "\nYou have already blocked this user\n\n";
		navigateScreen(INBOX_SCREEN);
	}

	else if (msg == "222") {
		cout << "\nYou have already been blocked by this user\n\n";
		navigateScreen(INBOX_SCREEN);
	}

	else if (msg == "260") {
		cout << "\nUnblock successfully\n\n";
		navigateScreen(INBOX_SCREEN);
	}

	else if (msg == "261") {
		cout << "\nYou have not blocked this username before\n\n";
		navigateScreen(INBOX_SCREEN);
	}

	/*// LOG OUT + INVITATION + ...
	else if (msg == "201") {
		cout << "\nYou not logged in yet\n";
		navigateScreen(curScreen);
	}*/

	// CREATE NEW GROUP
	// @NOT COMPLETE
	else if (msg.find("300") != string::npos) {
		cout << "\nCreate new group successfully ||| GROUP NAME: " << msg.substr(3) << "\n";
		// need to reload list of joined group...
		//joinedGroupList.clear();
		//cout << "Reload list of your joined group ..." << endl;
		curScreen = GROUP_SCREEN;
		navigateScreen(GROUP_SCREEN);



	}
	// CREATE NEW GROUP
	else if (msg == "301") {
		cout << "\nGroup name contains fobbiden character '/'\n";
		curScreen = GROUP_SCREEN;
		navigateScreen(GROUP_SCREEN);
	}

	// INVITE 
	else if (msg == "310") {
		cout << "\nSend inventation to a person to join your group successfully\n";
		curScreen = GROUP_SCREEN;
		navigateScreen(GROUP_SCREEN);
	}
	// INVITE + ...
	else if (msg == "312") {
		cout << "\nGroup ID not exist\n";
		curScreen = GROUP_SCREEN;
		navigateScreen(GROUP_SCREEN);
	}
	// INVITE 
	else if (msg == "313") {
		cout << "\nUsername already in this group\n";
		curScreen = GROUP_SCREEN;
		navigateScreen(GROUP_SCREEN);
	}
	// INVITE + LEAVEGROUP + ...
	else if (msg == "341") {
		cout << "\nYou are not in this group currently. Cannot invite anyone\n";
		curScreen = GROUP_SCREEN;
		navigateScreen(GROUP_SCREEN);
	}


	// LISTGROUP
	else if (msg.find("360") == 0) {
		string content = msg.substr(strlen("360") + 1);
		// load to client's map

		joinedGroupList.clear();
		while (content.find("/") != string::npos) {
			int found = content.find("/");

			string groupInfo = content.substr(0, found);
			int groupId = atoi(groupInfo.substr(0, groupInfo.find(" ")).c_str());
			string groupDisplayName = groupInfo.substr(groupInfo.find(" ") + 1);
			joinedGroupList.insert({ groupId, groupDisplayName });

			if (found + 1 < content.length())
				content = content.substr(found + 1);
			else
				content = "";
		}

		if (joinedGroupList.size() > 0) {

			cout << "\nList of your joined group:\n";
			for (auto it = joinedGroupList.begin(); it != joinedGroupList.end(); it++) {
				cout << it->first << " " << it->second << endl;
			}

			cout << "Choose your target groupId: "; cin >> groupId;
			curScreen = CHAT_GROUP_SCREEN;
			navigateScreen(CHAT_GROUP_SCREEN);

		}
		else {
			cout << "You not in any group!";
			curScreen = GROUP_SCREEN;
			navigateScreen(GROUP_SCREEN);
		}


	}

	// 
	// else if ...


	else {
		// msg = "999"
		cout << "\nBad request\n";
		navigateScreen(curScreen);
	}

}




void navigateScreen(SCREEN screen) {
	curScreen = screen;
	switch (screen) {
	case LOGIN_SCREEN:
		showLoginMenu();
		handleLoginMenu();
		break;
	case MAIN_SCREEN:
		showMainMenu();
		handleMainMenu();
		break;
	case CHAT_SCREEN:
		showListOnlineUsers();
		showChatMenu();
		handleChatMenu();
		break;
	case INBOX_SCREEN:
		showInboxMenu(userChatWith);
		handleInboxMenu();
		break;
	case CONVERSATION_SCREEN:
		handleConversation();
		break;
	case CHAT_GROUP_SCREEN:
		showChatGroupMenu();
		cout << "You choose groupId = " << groupId << endl;
		handleChatGroupMenu();
		break;
		//default:
		//	break;

	}
}



unsigned __stdcall recvThread(void* param) {
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
			receiveBuffer += string(buff);

			while (receiveBuffer.find("\r\n") != string::npos) {
				size_t found = receiveBuffer.find("\r\n");
				string message = receiveBuffer.substr(0, found); //message contains code received from server
				//std::cout << message; //display a message to user according to code received

				receiveBuffer = receiveBuffer.substr(found + 2); //cut the part of the received string that has been processed

				handleResponse(message);

				
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

	navigateScreen(curScreen);

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

