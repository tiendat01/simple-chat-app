#include <stdio.h>
#include <conio.h>
#include <process.h>
#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>
#include <vector>
#include <map>
#include <sstream>

#include "Protocol.h"
#include "UtilsFunction.h"

#pragma comment(lib, "ws2_32.lib")

using namespace std;


SOCKET clientSock;

CRITICAL_SECTION criticalSection;
string receiveBuffer;
SCREEN curScreen;

map<int, string> joinedGroupList; // for showing only in LISTGROUP req. Don't use 
int globalGroupId = -1;
string groupHistory;
string usernameLogin;

string userChatWith;
vector<string> onlineUsersList;
vector<string> conversationMessages;

void handleResponse(string msg);
void navigateScreen(SCREEN screen);


//void navigateScreen(SCREEN screen);



/// ################# SEND REQUEST SERVER  #################


/// PART 1: LOG IN, SIGN UP, LOG OUT

void login() {
	string username, password;
	cout << "Username: "; cin >> username; cin.ignore();
	cout << "Password: "; getline(cin, password);

	string req = LOGIN_REQ + string(DELIMITER1) + username + " " + password + ENDING_DELIMITER;
	int ret = Send(clientSock, req, req.length(), 0);

	usernameLogin = username;
}


void signup() {
	string username, password;
	cout << "Register new account: " << endl;
	cout << "Username: "; cin >> username; cin.ignore(); // ignore '\n'
	cout << "Password: "; getline(cin, password);

	string req = SIGNUP_REQ + string(DELIMITER1) + username + " " + password + ENDING_DELIMITER;
	int ret = Send(clientSock, req, req.length(), 0);
}


void logout() {
	string req = LOGOUT_REQ + string(ENDING_DELIMITER);
	int ret = Send(clientSock, req, req.length(), 0);
}


/// PART 2: INBOX CHAT ===> GIANG UPDATE THIS

// send request to get list of online users and show
void loadOnlineUsers() {
	sendProcessing(clientSock, ONLINE_REQ);
}

void inboxAnUser() {
	cout << "Enter username you want to chat with: ";
	cin >> userChatWith;
	sendProcessing(clientSock, CHECK_REQ, userChatWith);
}

void sendInboxMessage() {
	cout << "Enter message: ";
	cin.ignore();
	string message; getline(cin, message);
	sendProcessing(clientSock, SEND_REQ, userChatWith, message);
}

void showInboxMessages() {
	sendProcessing(clientSock, LOAD_CONVERSATION_REQ, userChatWith);
}

void blockUser() {
	sendProcessing(clientSock, BLOCK_REQ, userChatWith);
}

void unblockUser() {
	sendProcessing(clientSock, UNBLOCK_REQ, userChatWith);
}

/// ######### SHOW onlineUsersList VARIABLE ###############
void showListOnlineUsers() {
	cout << "\n\nUser Online:\n";
	for (int i = 0; i < onlineUsersList.size(); i++) cout << (i + 1) << " " << onlineUsersList[i] << "\n";
}


/// PART 3: GROUP CHAT (LINHVD + DATNT)


void showListJoinedGroup() {

	string req = LIST_JOINED_GROUP_REQ + string(ENDING_DELIMITER);
	int ret = Send(clientSock, req, req.length(), 0);

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
	cout << "ID\tGroup Name\n";
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
	int ret = Send(clientSock, req, req.length(), 0);
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

	string req = string(LIST_MEMBERS_GROUP) + string(DELIMITER1) + to_string(globalGroupId) + ENDING_DELIMITER;
	int ret = Send(clientSock, req, req.length(), 0);
}




void leaveGroup(int groupId) {
	string req = LEAVE_GROUP_REQ + string(" ") + to_string(groupId) + string(ENDING_DELIMITER);
	Send(clientSock, req, req.length(), 0);
}



void showGroupChatBox() {
	string req = LOAD_GROUP_MESSAGE + string(DELIMITER1) + to_string(globalGroupId) + ENDING_DELIMITER;
	int ret = Send(clientSock, req, req.length(), 0);
}


void sendNewMsg(string msg) {
	string req = GROUP_MESSAGE + string(DELIMITER1) + to_string(globalGroupId) + string(DELIMITER1) + msg + ENDING_DELIMITER;
	int ret = Send(clientSock, req, req.length(), 0);
}




/// ######### HANDLE ALL MENU CLI ###############


// LOGIN_SCREEN
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


// MAIN_SCREEN
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
		// inbox
	case 2:
		loadOnlineUsers();
		break;
		// group chat
	case 3:
		showListJoinedGroup();
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

// INBOX CHAT MENU
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

// IN CONVERSATION BETWEEN 2 USERS
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



// GROUP_SCREEN
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


// ONE_GROUP_SCREEN
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
		// go to group chat box
	case 3:
		curScreen = GROUP_CHAT_SCREEN;
		showGroupChatBox();
		break;

	default:
		break;
	}
}


// THIS FUNCTION IS UPDATED IN ANOTHER PROJECT: group-chat (by LinhVD)
void handleGroupChatBox() {
	system("cls");
	cout << groupHistory << endl;
	bool isOut = false;
	string msg;
	//while (!isOut) {
	cin.ignore(); getline(cin, msg);

	/*if (msg[0] == '1')
	break;*/
	if (msg.length() > 0)
		sendNewMsg(msg);
	//}


	showOneGroupMenu(joinedGroupList.at(globalGroupId).c_str());
	handleOneGroupMenu();
}




/// ################# HANDLE RESPONSE FROM SERVER #################

// need refactory and cut to Header.h
void handleResponse(string msg) {

	/// PART 1

	// LOGIN
	if (msg.find(LOGIN_SUCCESS) == 0) {
		cout << "\nLogin successfully\n";
		curScreen = MAIN_SCREEN;
		navigateScreen(MAIN_SCREEN);
		usernameLogin = msg.substr(msg.find(" ") + 1);
	}
	// LOGIN + INVITE
	else if (msg == ACCOUNT_NOT_FOUND) {
		cout << "\nUsername not exist\n";
		navigateScreen(curScreen);
	}
	// LOGIN
	else if (msg == INCORRECT_PASSWORD) {
		cout << "\nPassword incorrect\n";
		curScreen = LOGIN_SCREEN;
		navigateScreen(LOGIN_SCREEN);

	}
	// LOGIN
	else if (msg == ACCOUNT_LOGGED_IN) {
		cout << "\nAn account has been logged in already in this session!\n";
		curScreen = LOGIN_SCREEN;
		navigateScreen(LOGIN_SCREEN);

	}
	// LOGIN
	else if (msg == ACCOUNT_OTHER_SESSION) {
		cout << "\nThis account has been logged in already in other session!\n";
		curScreen = LOGIN_SCREEN;
		navigateScreen(LOGIN_SCREEN);

	}

	// SIGN UP
	else if (msg == SIGNUP_SUCCESS) {
		cout << "\nCreate new account sucessfully\n";
		curScreen = LOGIN_SCREEN;
		navigateScreen(LOGIN_SCREEN);
	}
	// SIGN UP
	else if (msg == ACCOUNT_EXISTED) {
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
	else if (msg == LOGOUT_SUCESS) {
		cout << "\nLog out successfully\n";
		curScreen = LOGIN_SCREEN;
		navigateScreen(LOGIN_SCREEN);
	}

	// LOG OUT + INVITATION + ...
	else if (msg == ACCOUNT_NOT_LOGGED_IN) {
		cout << "\nYou not logged in yet\n";
		navigateScreen(curScreen);
	}


	/// PART 2: CHAT 1-1


	// LIST ONLINE USERS
	else if (msg.substr(0, 3) == LIST_ONLINE) {
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

	// CHECK IF USER EXISTS
	else if (msg == USERNAME_EXIST) {
		navigateScreen(INBOX_SCREEN);
	}

	else if (msg == USERNAME_NOT_EXIST) {
		cout << "\nThis username doesn't exist\n\n";
		loadOnlineUsers();
	}

	// SEND MESSAGE INBOX
	else if (msg.substr(0, 3) == SEND_SUCCESS) {
		cout << "\n\n";
		showInboxMessages();
	}

	else if (msg.substr(0, 3) == CONVERSATION_HISTORY) {
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

	else if (msg.substr(0, 3) == RECEICE_MESSAGE) {

		string content = msg.substr(4);
		string username = content.substr(0, content.find(" "));
		string newMessage = content.substr(content.find(" ") + 1);
		if (curScreen == CONVERSATION_SCREEN && userChatWith == username) {
			cout << newMessage << "\n";
		}
	}

	// BLOCK
	else if (msg == BLOCK_SUCCESS) {
		cout << "\nBlock successfully\n\n";
		navigateScreen(INBOX_SCREEN);
	}

	else if (msg == BLOCK_BEFORE) {
		cout << "\nYou have already blocked this user\n\n";
		navigateScreen(INBOX_SCREEN);
	}

	else if (msg == BLOCKED_BEFORE) {
		cout << "\nYou have already been blocked by this user\n\n";
		navigateScreen(INBOX_SCREEN);
	}

	// UNBLOCK
	else if (msg == UNBLOCK_SUCCESS) {
		cout << "\nUnblock successfully\n\n";
		navigateScreen(INBOX_SCREEN);
	}

	else if (msg == NOT_BLOCK_BEFORE) {
		cout << "\nYou have not blocked this username before\n\n";
		navigateScreen(INBOX_SCREEN);
	}



	/// PART 3 - GROUP CHAT

	// CREATE NEW GROUP
	else if (msg.find(CREATE_GROUP_SUCCESS) != string::npos) {
		cout << "\nCreate new group successfully ||| GROUP NAME: " << msg.substr(3) << "\n";

		// need to reload list of joined group... ???
		showListJoinedGroup();
	}

	// CREATE NEW GROUP
	else if (msg == FOBBIDEN_TOKEN) {
		cout << "\nGroup name contains fobbiden character '/'\n";

		// come back to curScreen = GROUP_MENU
		showListJoinedGroup();

	}


	// INVITE 
	else if (msg.find(INVITE_SUCCESS) != string::npos) {
		cout << "\nSend inventation to a person to join your group successfully\n";
		curScreen = GROUP_SCREEN;
		navigateScreen(GROUP_SCREEN);
	}
	// INVITE + LEAVE GROUP
	else if (msg == GROUP_NOT_EXIST) {
		cout << "\nGroup ID not exist\n";
		navigateScreen(curScreen);
	}
	// INVITE 
	else if (msg == USER_ALREADY_IN_GROUP) {
		cout << "\nUsername already in this group\n";
		curScreen = GROUP_SCREEN;
		navigateScreen(GROUP_SCREEN);
	}
	// INVITE + LEAVEGROUP + ...
	else if (msg == NOT_IN_GROUP) {
		cout << "\nYou are not in this group currently.\n";
		navigateScreen(curScreen);
	}
	// INVITE
	else if (msg == DUPLICATE_INVITATION) {
		cout << "\nYou have already sent invitation to this user to join this group\n";
		curScreen = GROUP_SCREEN;
		navigateScreen(GROUP_SCREEN);
	}

	// GET INVITATION
	else if (msg.find(GET_INVITATION_SUCESS) == 0) {

		string content = msg.substr(strlen(GET_INVITATION_SUCESS) + 1);
		cout << "Invitation to group from other user:\n";

		if (content.find("/") == string::npos) {
			cout << "You don't have any invitation from anyone! Feel free :))" << endl;
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
	else if (msg.find(ACCEPT_SUCCESS) == 0) {
		string content = msg.substr(strlen(ACCEPT_SUCCESS) + 1);
		int groupId = atoi(content.substr(0, content.find(" ")).c_str());
		string groupDisplayName = content.substr(content.find(" ") + 1);
		cout << "\nAccept and join groupId: " << groupId << " name: " << groupDisplayName << " successfully\n";

		showListJoinedGroup();
	}

	// DENY invitation
	else if (msg.find(DENY_SUCCESS) == 0) {

		string content = msg.substr(strlen(DENY_SUCCESS) + 1);
		int groupId = atoi(content.substr(0, content.find(" ")).c_str());
		string groupDisplayName = content.substr(content.find(" ") + 1);
		cout << "\nDeny to join groupId: " << groupId << " name: " << groupDisplayName << " successfully\n";

		curScreen = GROUP_SCREEN;
		navigateScreen(GROUP_SCREEN);
	}

	else if (msg == NOT_INVITED_TO_THIS_GROUP) {
		cout << "\nYou aren't invited to this group by anyone!\n";
		curScreen = GROUP_SCREEN;
		navigateScreen(GROUP_SCREEN);
	}

	else if (msg == ALREADY_IN_THIS_GROUP) {
		cout << "\nYou are in this group already\n";
		curScreen = GROUP_SCREEN;
		navigateScreen(GROUP_SCREEN);
	}





	// LISTGROUP
	else if (msg.find(LIST_GROUP_SUCCESS) == 0) {
		string content = msg.substr(strlen(LIST_GROUP_SUCCESS) + 1);
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
			cout << "ID\tGroup Name\n";
			for (auto it = joinedGroupList.begin(); it != joinedGroupList.end(); it++) {
				cout << it->first << "\t" << it->second << endl;
			}

			if (curScreen == MAIN_SCREEN)
				curScreen = GROUP_SCREEN;

			navigateScreen(curScreen);
		}
		else {
			cout << "You not in any group!";
			curScreen = GROUP_SCREEN;
			navigateScreen(GROUP_SCREEN);
		}

		// After get all join group ==> navigate to respective screen


	}

	// LEAVE GROUP
	else if (msg == LEAVE_GROUP_SUCCESS) {
		cout << "\nLeave group sucessfully\n";
		showListJoinedGroup();
	}

	// SHOW LIST OF MEMBERS IN GROUP
	else if (msg.substr(0, 3) == LISTMEMBERS_SUCCESS) {
		msg = msg.substr(msg.find(" ") + 1);
		cout << msg << endl;

		showOneGroupMenu(joinedGroupList.at(globalGroupId).c_str());
		handleOneGroupMenu();
	}


	// SHOW GROUP CHAT MESSSAGES
	else if (msg.substr(0, 3) == LOADMSG_SUCCESS) {
		msg = msg.substr(msg.find(" ") + 1);
		msg = msg.substr(msg.find(" ") + 1);
		groupHistory = msg;

		curScreen = GROUP_CHAT_SCREEN;
		navigateScreen(GROUP_CHAT_SCREEN);
	}


	// SUCESS SEND TO GROUP
	else if (msg.substr(0, 3) == GROUPMESSAGE_SUCCESS) {
		msg = msg.substr(msg.find(" ") + 1);
		string groupId = msg.substr(0, msg.find(" "));
		msg = msg.substr(msg.find(" ") + 1);
		string username = msg.substr(0, msg.find(" "));
		msg = msg.substr(msg.find(" ") + 1);
		string newMsg = msg.substr(0);

		if (groupId == to_string(globalGroupId) && username == usernameLogin)
			groupHistory += newMsg;

		handleGroupChatBox();

	}


	// 
	else if (msg.substr(0, 3) == RECEIVE_MESSAGE_GROUP) {
		//cout << "Server return " << msg << endl;
		msg = msg.substr(msg.find(" ") + 1);
		string groupId = msg.substr(0, msg.find(" "));
		string newMessage = msg.substr(msg.find(" ") + 1);

		if (groupId == to_string(globalGroupId)) {
			groupHistory += newMessage;
		}
	}

	// else if ...


	else {
		cout << "\nBad request\n"; // msg = "999"
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
	case GROUP_SCREEN:
		showGroupMenu();
		handleGroupMenu();
		break;

	case ONE_GROUP_SCREEN:
		int targetGroupId;
		cout << "Choose your target groupId: "; cin >> targetGroupId;
		if (joinedGroupList.find(targetGroupId) == joinedGroupList.end()) {
			cout << "You are not in this group chat" << endl;
			break;
		}

		globalGroupId = targetGroupId;

		showOneGroupMenu(joinedGroupList.at(globalGroupId).c_str());
		handleOneGroupMenu();
		break;

	case GROUP_CHAT_SCREEN:
		handleGroupChatBox();
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

			// stream handle
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