#pragma once
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>

#define BUFF_SIZE 2048
#define ENDING_DELIMITER "\r\n"
using namespace std;

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
	CHAT_SCREEN,
	INBOX_SCREEN,
	CONVERSATION_SCREEN,
	GROUP_SCREEN,
	CHAT_GROUP_SCREEN
};

/**
@function: isNumeric - check if char array contains only digits ?

@param: s - A pointer to a input string

@return:
true if char array contains only digits
false if not
**/
int isNumeric(char* s);




/**
@function: showMainMenu

@param: none

@return: void
**/
void showMainMenu();


void showChatMenu();


void showLoginMenu();



void showInboxMenu(string userChatWith);



void showGroupMenu();





void showChatGroupMenu();



void navigateScreen(SCREEN screen);


