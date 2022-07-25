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
#include <sstream>


#pragma comment(lib, "ws2_32.lib")

using namespace std;

#define LOGIN_REQ "LOGIN"
#define SIGNUP_REQ "REGISTER"
#define LOGOUT_REQ "LOGOUT"

#define CREATE_GROUP_REQ "CREATEGROUP"
#define LEAVE_GROUP_REQ "LEAVEGROUP"

#define INVITE_REQ "INVITE"
#define LIST_JOINED_GROUP_REQ "LISTGROUP"
#define GET_INVITATION_REQ "GETINVITATION"
#define ACCEPT_REQ "ACCEPT"
#define DENY_REQ "DENY"
#define DELIMITER " "

#define ACCOUNT_LEAVE_GROUP "LEAVE" 
#define LIST_GROUP "LISTGROUP"
#define LIST_MEMBERS_GROUP "LISTMEMBERS" 
#define LOAD_MESSAGE  "LOADMESSAGE"
#define MESSAGE "MESSAGE"

SOCKET clientSock;

CRITICAL_SECTION criticalSection;
string receiveBuffer;
SCREEN curScreen = LOGIN_SCREEN;

map<int, string> joinedGroupList; // for showing only in LISTGROUP req. Don't use 
int globalGroupId = -1;
string groupHistory;
string usernameLogin;


//vector<string> onlineUsersList;
//int codescreen = 0;
//string listmessage;

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
	string choice;
	do {
		cout << "Your choice: ";
		cin >> choice;
		//cin.ignore();
	} while (choice.at(0) > '0' + maxChoice || choice.at(0) < '0' || choice.length() != 1);
	return choice.at(0) - 48;
}





/// ################# SEND REQUEST SERVER  #################



/// PART 1: LOG IN, SIGN UP, LOG OUT

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


/// PART 2: INBOX CHAT

// send request to get list of online users and show
void showListOnlineUsers() {
	
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


/// PART 3: GROUP CHAT


void showListJoinedGroup() {

	string req = LIST_JOINED_GROUP_REQ + string(ENDING_DELIMITER);
	EnterCriticalSection(&criticalSection);
	int ret = Send(clientSock, req, req.length(), 0); // return "360"
	LeaveCriticalSection(&criticalSection);

}


void createNewGroup() {
	string groupDisplayName; 
	cout << "Enter your new Group name (forbidden character '/'): "; 
	cin.ignore(); getline(cin, groupDisplayName);

	string req = CREATE_GROUP_REQ + string(" ") + groupDisplayName + string(ENDING_DELIMITER);
	int ret = Send(clientSock, req, req.length(), 0);

}


void invite() {

	// Must ensure the joinedGroupList be updated first
	cout << "\nList of your joined group:\n";
	for (auto it = joinedGroupList.begin(); it != joinedGroupList.end(); it++) {
		cout << it->first << " " << it->second << endl;
	}

	string destUsername;
	int groupId;
	cout << "Enter your joined group id: "; cin >> groupId; 
	cout << "Enter username of person you want to invite: "; cin >> destUsername;

	string req = INVITE_REQ + string(" ") + destUsername + " " + to_string(groupId) + string(ENDING_DELIMITER);
	int ret = Send(clientSock, req, req.length(), 0);

}

void getInvitation() {
	string req = GET_INVITATION_REQ + string(ENDING_DELIMITER);
	EnterCriticalSection(&criticalSection);
	int ret = Send(clientSock, req, req.length(), 0);
	LeaveCriticalSection(&criticalSection);
}



void responseInvitation() {

	// accept or deny
	string groupIdStr, choiceStr;
	int groupId, choice;
	cout << "Enter any groupId in your invitations: "; cin >> groupIdStr; groupId = atoi(groupIdStr.c_str());
	cout << "Choose Operation with this invitation (1: Accept / 0: Deny / Default: Do nothing and back): "; 
	cin.ignore(); getline(cin, choiceStr); choice = choiceStr[0] - 48;

	string req;
	switch (choice)
	{
	case 0: 
		req = DENY_REQ + string(" ") + to_string(groupId) + string(ENDING_DELIMITER);
		Send(clientSock, req, req.length(), 0);
		break;
	case 1:
		req = ACCEPT_REQ + string(" ") + to_string(groupId) + string(ENDING_DELIMITER);
		Send(clientSock, req, req.length(), 0);
		break;
	default:
		curScreen = GROUP_SCREEN;
		navigateScreen(GROUP_SCREEN);
		break;
	}
}

void showListMemberGroup() {

	string req = string(LIST_MEMBERS_GROUP) + string(DELIMITER) + to_string(globalGroupId) + ENDING_DELIMITER;
	int ret = Send(clientSock, req, req.length(), 0);
}




void leaveGroup(int groupId) {
	string req = LEAVE_GROUP_REQ + string(" ") + to_string(groupId) + string(ENDING_DELIMITER);
	Send(clientSock, req, req.length(), 0);
}



void showGroupChatBox() {
	string req = LOAD_MESSAGE + string(DELIMITER) + to_string(globalGroupId) + ENDING_DELIMITER;
	int ret = Send(clientSock, req, req.length(), 0);
}




void chatInGroupBox() {

	string msg;
	cout << "You: ";
	int messageLen = 0;
	do {
		fflush(stdin);
		char buff[BUFF_SIZE];
		memset(buff, 0, BUFF_SIZE);
		gets_s(buff, BUFF_SIZE);
		messageLen = strlen(buff);
		if (messageLen > 0) {
			string req = "MESSAGE" + string(DELIMITER) + to_string(globalGroupId) + string(DELIMITER) + string(buff) + ENDING_DELIMITER;
			int ret = Send(clientSock, req, req.length(), 0);
		}
	}while(!messageLen);

}



/// ######### HANDLE ALL MENU GUI ###############



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
	// logout and back (oke)
	case 1:
		logout();
		break;
	// inbox (not)
	case 2:
		showListOnlineUsers();
		break;
	// group chat (not)
	case 3:
		showListJoinedGroup();
		break;
	default:
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
		getInvitation();
		break;
		// choose a group by groupid
	case 4:
		curScreen = ONE_GROUP_SCREEN;
		showListJoinedGroup();
		break;
	}

}




void handleOneGroupMenu() {
	
	int choice = getChoice(3);
	switch (choice)
	{
		// back to GROUP SCREEN
	case 0:
		curScreen = GROUP_SCREEN;
		navigateScreen(GROUP_SCREEN);
		break;
		// list members in group
	case 1:
		showListMemberGroup();
		break;
		// leave group
	case 2:
		curScreen = ONE_GROUP_SCREEN;
		leaveGroup(globalGroupId);
		break;
	case 3:
		curScreen = GROUP_CHAT_SCREEN;
		navigateScreen(GROUP_CHAT_SCREEN);
		break;

	default:
		break;
	}
}



void handleGroupChatBox() {
	int choice = getChoice(2);
	switch (choice)
	{
	case 0:
		curScreen = ONE_GROUP_SCREEN;
		navigateScreen(ONE_GROUP_SCREEN);
		break;
	case 1:
		chatInGroupBox();
		break;
	case 2:
		showGroupChatBox();
		break;
	default:
		break;
	}
}



void displayGroupChatBox() {
	printf("\n\n-------------- %s --------------\n", joinedGroupList.at(globalGroupId).c_str());
	printf("Press '1' to leave\n\n");
	cout << groupHistory << endl;
	string leave; cin >> leave;
	if (leave == "1") {
		navigateScreen(GROUP_CHAT_SCREEN);
	}

}




/// ################# HANDLE RESPONSE FROM SERVER #################

// need refactory and cut to Header.h
void handleResponse(string msg) {

	//cout << "Receive from server: " + msg << endl;

	// LOGIN
	if (msg.find("100") == 0) {
		cout << "\nLogin successfully\n";
		curScreen = MAIN_SCREEN;
		navigateScreen(MAIN_SCREEN);
		usernameLogin = msg.substr(msg.find(" ") + 1);
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

	// LOG OUT + INVITATION + ...
	else if (msg == "201") {
		cout << "\nYou not logged in yet\n";
		navigateScreen(curScreen);
	}




	// CREATE NEW GROUP
	else if (msg.find("300") != string::npos) {
		cout << "\nCreate new group successfully ||| GROUP NAME: " << msg.substr(3) << "\n";

		// need to reload list of joined group... ???
		showListJoinedGroup();
	}

	// CREATE NEW GROUP
	else if (msg == "301") {
		cout << "\nGroup name contains fobbiden character '/'\n";
		
		// come back to curScreen = GROUP_MENU
		showListJoinedGroup();

	}




	// INVITE 
	else if (msg.find("310") != string::npos) {
		cout << "\nSend inventation to a person to join your group successfully\n";
		curScreen = GROUP_SCREEN;
		navigateScreen(GROUP_SCREEN);
	}
	// INVITE + LEAVE GROUP
	else if (msg == "312") {
		cout << "\nGroup ID not exist\n";
		navigateScreen(curScreen);
	}
	// INVITE 
	else if (msg == "313") {
		cout << "\nUsername already in this group\n";
		curScreen = GROUP_SCREEN;
		navigateScreen(GROUP_SCREEN);
	}
	// INVITE + LEAVEGROUP + ...
	else if (msg == "341") {
		cout << "\nYou are not in this group currently.\n";
		navigateScreen(curScreen);
	}
	// INVITE
	else if (msg == "314") {
		cout << "\nYou have already sent invitation to this user to join this group\n";
		curScreen = GROUP_SCREEN;
		navigateScreen(GROUP_SCREEN);
	}

	// GET INVITATION
	else if (msg.find("320") == 0) {

		string content = msg.substr(strlen("320") + 1);
		cout << "Invitation to group from other user:\n";

		if (content.find("/") == string::npos) {
			cout << "You don't have any invitation from anyone! Feel free :))";
			curScreen = GROUP_SCREEN;
			navigateScreen(GROUP_SCREEN);
		}
		else {
			while (content.find("/") != string::npos) {
				int found = content.find("/");
				string invitation = content.substr(0, found);
				int first = invitation.find(" ");
				int last = invitation.rfind(" ");

				int groupId = atoi(invitation.substr(0, first).c_str());
				string fromUsername = invitation.substr(last + 1);
				string groupDisplayName = invitation.substr(first + 1, invitation.length() - first - 1 - fromUsername.length() - 1);
				cout << "To groupId " << groupId << " Group Name: " << groupDisplayName << " From " << fromUsername << endl;

				if (found + 1 < content.length())
					content = content.substr(found + 1);
				else
					content = "";

			}
			responseInvitation();
		}
	}

	// ACCEPT invitation
	else if (msg.find("330") == 0) {
		string content = msg.substr(strlen("330") + 1);
		int groupId = atoi(content.substr(0, content.find(" ")).c_str());
		string groupDisplayName = content.substr(content.find(" ") + 1);
		cout << "\nAccept and join groupId: " << groupId << " name: " << groupDisplayName << " successfully\n";

		showListJoinedGroup();
	}

	// DENY invitation
	else if (msg.find("333") == 0) {

		string content = msg.substr(strlen("333") + 1);
		int groupId = atoi(content.substr(0, content.find(" ")).c_str());
		string groupDisplayName = content.substr(content.find(" ") + 1);
		cout << "\nDeny to join groupId: " << groupId << " name: " << groupDisplayName << " successfully\n";

		curScreen = GROUP_SCREEN;
		navigateScreen(GROUP_SCREEN);
	}

	else if (msg == "331") {
		cout << "\nYou aren't invited to this group by anyone!\n";
		curScreen = GROUP_SCREEN;
		navigateScreen(GROUP_SCREEN);
	}

	else if (msg == "332") {
		cout << "\nYou are in this group already\n";
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

		}
		else {
			cout << "You not in any group!";
		}

		// After get all join group ==> navigate to respective screen

		if (curScreen == MAIN_SCREEN)
			curScreen = GROUP_SCREEN;

		navigateScreen(curScreen);
	}

	// LEAVE GROUP
	else if (msg == "350") {
		cout << "\nLeave group sucessfully\n";
		showListJoinedGroup();
	}
	
	// SHOW LIST OF MEMBERS IN GROUP
	else if (msg.substr(0, 3) == "370") {
		msg = msg.substr(msg.find(" ") + 1);
		cout << msg << endl;

		showOneGroupMenu(joinedGroupList.at(globalGroupId).c_str());
		handleOneGroupMenu();
	}


	// SHOW GROUP CHAT MESSSAGES
	else if (msg.substr(0, 3) == "380") {
		msg = msg.substr(msg.find(" ") + 1);
		msg = msg.substr(msg.find(" ") + 1);
		groupHistory = msg;
		curScreen = LIST_MESSAGE_SCREEN;
		navigateScreen(LIST_MESSAGE_SCREEN);
	}


	// SUCESS SEND TO GROUP
	else if (msg.substr(0, 3) == "390") {
		msg = msg.substr(msg.find(" ") + 1);
		string groupId = msg.substr(0, msg.find(" "));
		msg = msg.substr(msg.find(" ") + 1);
		string username = msg.substr(0, msg.find(" "));

		if (groupId == to_string(globalGroupId) && username == usernameLogin)
			showGroupChatBox();
	}


	else if (msg.substr(0, 3) == "381") {
		cout << "Server return " << msg << endl;
		msg = msg.substr(msg.find(" ") + 1);
		string groupId = msg.substr(0, msg.find(" "));
		string newMessage = msg.substr(msg.find(" ") + 1);

		cout << groupId << " " << curScreen;

		if (groupId == to_string(globalGroupId) && curScreen == LIST_MESSAGE_SCREEN) {
			cout << newMessage << endl;
		}
	}

	// else if ...


	else {
		// msg = "999"
		cout << "\nBad request\n";
		navigateScreen(curScreen);
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

		// GIANG UPDATE THIS 

		break;
	case GROUP_SCREEN:
		showGroupMenu();
		handleGroupMenu();
		break;


	case ONE_GROUP_SCREEN:
		repeat:
		int targetGroupId;
		cout << "Choose your target groupId: "; cin >> targetGroupId;
		if (joinedGroupList.find(targetGroupId) == joinedGroupList.end())
			goto repeat;

		globalGroupId = targetGroupId;

		showOneGroupMenu(joinedGroupList.at(globalGroupId).c_str());
		handleOneGroupMenu();
		break;

	case GROUP_CHAT_SCREEN:
		showGroupChatMenu(joinedGroupList.at(globalGroupId).c_str());
		handleGroupChatBox();
		break;

	case LIST_MESSAGE_SCREEN:
		displayGroupChatBox();
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
			receiveBuffer += string(buff);

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

