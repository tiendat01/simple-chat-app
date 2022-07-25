/*
Implementation for ClientHeader.h
*/

#include <stdio.h>
#include <conio.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include "Header.h"


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

void showOneGroupMenu(const char* groupDisplayName) {

	// choose group by id ...

	printf("\n\n-------------- %s -----------------\n", groupDisplayName);
	printf("Choose option: Press keyboard a number:\n");
	printf("0. Back\n");
	printf("1. List all members in this group\n");
	printf("2. Leave this group\n");
	printf("3. Go to group chat menu of this group\n");
	
}


// groupChatBox
void showGroupChatMenu(const char* groupDisplayName) {
	printf("\n\n ------------------ %s -----------------------\n", groupDisplayName);
	printf("Choose option: Press keyboard a number:\n");
	printf("0. Back\n");
	printf("1. Send a message to group %s\n", groupDisplayName);
	printf("2. Show group chat box\n");
}
