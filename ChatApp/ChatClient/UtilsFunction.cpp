/*
Implementation for ClientHeader.h
*/

#include <stdio.h>
#include <conio.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>

#include "Protocol.h"
#include "UtilsFunction.h"


using namespace std;

/// ############## UTILS FUNCTION ##############


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

/* The recv() wrapper function */
int Receive(SOCKET s, char *buff, int size, int flags) {
	int n;

	n = recv(s, buff, size, flags);
	if (n == SOCKET_ERROR)
		printf("Error %d: Cannot receive data.\n", WSAGetLastError());
	return n;
}

/* The send() wrapper function*/
int Send(SOCKET s, string request, int size, int flags) {
	int n;
	const char* buff = request.c_str();
	n = send(s, buff, size, flags);
	if (n == SOCKET_ERROR)
		printf("Error %d: Cannot send data.\n", WSAGetLastError());

	return n;
}

void sendProcessing(SOCKET clientSock, string header) {
	string req = header + ENDING_DELIMITER;
	int ret = Send(clientSock, req, req.length(), 0);
}

void sendProcessing(SOCKET clientSock, string header, string s1) {
	string req = header + " " + s1 + ENDING_DELIMITER;
	int ret = Send(clientSock, req, req.length(), 0);
}

void sendProcessing(SOCKET clientSock, string header, string s1, string s2) {
	string req = header + " " + s1 + " " + s2 + ENDING_DELIMITER;
	int ret = Send(clientSock, req, req.length(), 0);
}

int getChoice(int maxChoice) {
	string choice;
	do {
		cout << "Your choice: ";
		cin >> choice;
	} while (choice.at(0) > '0' + maxChoice || choice.at(0) < '0' || choice.length() != 1);
	return choice.at(0) - 48;
}

int isNumeric(char *s) {
	while (*s != '\0') {
		if (!isdigit(*s)) {
			return 0;
		}
		s++;
	}
	return 1;
}


void showLoginMenu() {
	printf("\n\n-------------- LOGIN MENU -----------------\n");
	printf("Choose option: Press keyboard a number:\n");
	printf("0. Exit\n");
	printf("1. Login\n");
	printf("2. Signup\n");
}


void showMainMenu() {
	printf("\n\n-------------- MAIN MENU -----------------\n");
	printf("Choose option: Press keyboard a number:\n");
	printf("0. Logout and exit\n");
	printf("1. Logout\n");
	printf("2. Inbox\n");
	printf("3. Group chat\n");
}


void showChatMenu() {
	printf("\n\n-------------- CHAT MENU -----------------\n");
	printf("Choose option: Press keyboard a number:\n");
	printf("0. Back\n");
	printf("1. Chat with friends by username\n");
}

void showInboxMenu(string userChatWith) {
	printf("\n\n-------------- INBOX MENU -----------------\n");
	cout << "You are in conversation with user " + userChatWith + "\n";
	printf("Choose option: Press keyboard a number:\n");
	printf("0. Back\n");
	printf("1. Send a new message\n");
	printf("2. Show messages\n");
	printf("3. Block\n");
	printf("4. Unblock\n");
}

void showGroupMenu() {
	// list of group ...

	printf("\n\n-------------- GROUP MENU -----------------\n");
	printf("Choose option: Press keyboard a number:\n");
	printf("0. Back\n");
	printf("1. Create new group chat\n");
	printf("2. Invite user to your joined group\n");
	printf("3. Accept/Deny available invitation to join a group\n");
	printf("4. Choose a group by groupId\n"); // navigate to a specific groupId

}

void showMessageHistory(string userChatWith) {
	printf("\n\n-------------- %s --------------\n", userChatWith.c_str());
	printf("Press '1' to leave\n");
}

void showOneGroupMenu(const char* groupDisplayName) {

	// choose group by id ...

	printf("\n\n-------------- %s -----------------\n", groupDisplayName);
	printf("Choose option: Press keyboard a number:\n");
	printf("0. Back\n");
	printf("1. List all members in this group\n");
	printf("2. Leave this group\n");
	printf("3. Go to group chat menu of this group\n");
	printf("4. Send a message to group (this function has some bug, use LinhVD's version)\n");
}



