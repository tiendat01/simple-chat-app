#pragma once
#include <WinSock2.h>
#include <WS2tcpip.h>

#define BUFF_SIZE 2048
#define ENDING_DELIMITER "\r\n"


// eg:
/**
@function: handle - process the sender and receiver of the client programme

@param:
clientSock: socket connect to current client
msgType: type of message to handle
isExit: value to check if user want to exit and logout the programme

@return: void
**/
void handle(SOCKET clientSock, int msgType, int isExit);

enum SCREEN
{
	LOGIN_SCREEN,
	MAIN_SCREEN,
	INBOX_SCREEN,
	GROUP_SCREEN,
	CHAT_GROUP_SCREEN,
	CHAT_GROUP_BOX_SCREEN,
	LIST_MESSAGE_SCREEN
};

/**
@function: isNumeric - check if char array contains only digits ?

@param: s - A pointer to a input string

@return:
true if char array contains only digits
false if not
**/
int isNumeric(char *s);




/**
@function: showMainMenu

@param: none

@return: void
**/
void showMainMenu();



void showLoginMenu();



void showInboxMenu();



void showGroupMenu();



void showChatGroupMenu();



void stopConversationDialog();



void navigateScreen(SCREEN screen);




void showListMessageGroupChat();




void showGroupChatBox(const char* groupDisplayName);