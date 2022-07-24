/*
Implementation for ClientHeader.h
*/

#include <stdio.h>
#include <conio.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include "Header.h"
#include <iostream>
#include <string>

using namespace std;


int isNumeric(char* s) {
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
	printf("0. Logout and exit\n"); // disconnect
	printf("1. Logout\n"); // back to LOGIN MENU
	printf("2. Inbox\n");
	printf("3. Group chat\n");
}


void showInboxMenu() {
	// list of online users .... here
	printf("\n\n-------------- INBOX MENU -----------------\n");
	printf("Choose option: Press keyboard a number:\n");
	printf("0. Back\n");
	printf("1. Chat with online friends by username\n");
}

void showChatMenu() {
	printf("\n\n-------------- CHAT MENU -----------------\n");
	printf("Choose option: Press keyboard a number:\n");
	printf("0. Back\n");
	printf("1. Chat with online friends by username\n");
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
	printf("\n\n-------------- GROUP MENU -----------------\n");
	printf("Choose option: Press keyboard a number:\n");
	printf("0. Back\n");
	printf("1. Create new group chat\n");
	printf("2. Invite user to your joined group\n");
	printf("3. Accept/Deny available invitation to join a group\n");
	printf("4. Chat in group by group id\n"); // navigate to CHAT GROUP MENU

}

void showMessageHistory(string userChatWith) {
	printf("\n\n-------------- %s --------------\n", userChatWith.c_str());
	printf("Press '1' to leave\n");
}

void showChatGroupMenu() {
	// list of group ...

	// choose group by id ...

	printf("\n\n-------------- CHAT GROUP MENU -----------------\n");
	printf("Choose option: Press keyboard a number:\n");
	printf("0. Back\n");
	printf("1. List online members in group by groupId\n");
	printf("2. Chat in this group\n");
	printf("3. Leave group chat by group id\n");
}

