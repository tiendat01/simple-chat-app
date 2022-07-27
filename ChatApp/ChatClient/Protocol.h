#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>
#include <iostream>

using namespace std;

enum SCREEN
{
	LOGIN_SCREEN,
	MAIN_SCREEN,
	CHAT_SCREEN,
	INBOX_SCREEN,
	CONVERSATION_SCREEN,
	GROUP_SCREEN,
	ONE_GROUP_SCREEN,
	GROUP_CHAT_SCREEN
};

#define BUFF_SIZE 2048
#define ENDING_DELIMITER "\r\n"
#define DELIMITER1 " "
#define DELIMITER2 "/"

#define LOGIN_REQ "LOGIN"
#define SIGNUP_REQ "REGISTER"
#define LOGOUT_REQ "LOGOUT"

#define LIST_JOINED_GROUP_REQ "LISTGROUP"
#define CREATE_GROUP_REQ "CREATEGROUP"
#define LEAVE_GROUP_REQ "LEAVEGROUP"

#define ONLINE_REQ "LISTONLINE" 
#define CHECK_REQ "CHECK"
#define BLOCK_REQ "BLOCK"
#define UNBLOCK_REQ "UNBLOCK"
#define SEND_REQ "SEND"
#define LOAD_CONVERSATION_REQ "LOADCONVERSATION"

#define INVITE_REQ "INVITE"
#define GET_INVITATION_REQ "GETINVITATION"
#define ACCEPT_REQ "ACCEPT"
#define DENY_REQ "DENY"


#define LIST_MEMBERS_GROUP "LISTMEMBERS" 
#define LOAD_GROUP_MESSAGE  "LOADGROUPMESSAGE"
#define GROUP_MESSAGE "GROUPMESSAGE"




// response code
#define BAD_REQUEST "999"
#define ACCOUNT_NOT_LOGGED_IN "201"


#define LOGIN_SUCCESS "100"
#define ACCOUNT_NOT_FOUND "101"
#define INCORRECT_PASSWORD "102"
#define ACCOUNT_LOGGED_IN "103"
#define ACCOUNT_OTHER_SESSION "104"
#define SIGNUP_SUCCESS "110"
#define ACCOUNT_EXISTED "111"
#define LOGOUT_SUCESS "120"


#define LIST_ONLINE "200"
#define USERNAME_EXIST "210"
#define USERNAME_NOT_EXIST "211"
#define SEND_SUCCESS "220"
#define BLOCK_BEFORE "221"
#define BLOCKED_BEFORE "222"
#define CONVERSATION_HISTORY "230"
#define RECEICE_MESSAGE "240"
#define BLOCK_SUCCESS "250"
#define UNBLOCK_SUCCESS "260"
#define NOT_BLOCK_BEFORE "261"

#define CREATE_GROUP_SUCCESS "300"
#define FOBBIDEN_TOKEN "301"
#define LEAVE_GROUP_SUCCESS "350"
#define LIST_GROUP_SUCCESS "360"
#define GET_INVITATION_SUCESS "320"
#define GROUP_NOT_EXIST "312"
#define NOT_IN_GROUP "341"
#define USER_ALREADY_IN_GROUP "313"
#define DUPLICATE_INVITATION "314"
#define INVITE_SUCCESS "310"
#define ACCEPT_SUCCESS "330"
#define NOT_INVITED_TO_THIS_GROUP "331"
#define ALREADY_IN_THIS_GROUP "332"
#define DENY_SUCCESS "333"
#define LISTMEMBERS_SUCCESS "370"
#define LOADMSG_SUCCESS "380"
#define RECEIVE_MESSAGE_GROUP "381"
#define GROUPMESSAGE_SUCCESS "390"




















//
//#include <WinSock2.h>
//#include <WS2tcpip.h>
//#include <string>
//
//#define BUFF_SIZE 2048
//#define ENDING_DELIMITER "\r\n"
//#define DELIMITER " "
//
//
//
//// eg:
///**
//@function: handle - process the sender and receiver of the client programme
//
//@param:
//clientSock: socket connect to current client
//msgType: type of message to handle
//isExit: value to check if user want to exit and logout the programme
//
//@return: void
//**/
//void handle(SOCKET clientSock, int msgType, int isExit);
//
//enum SCREEN
//{
//	LOGIN_SCREEN,
//	MAIN_SCREEN,
//	INBOX_SCREEN,
//	GROUP_SCREEN,
//	ONE_GROUP_SCREEN,
//	GROUP_CHAT_SCREEN
//};
//
///**
//@function: isNumeric - check if char array contains only digits ?
//
//@param: s - A pointer to a input string
//
//@return:
//true if char array contains only digits
//false if not
//**/
//int isNumeric(char *s);
//
//
//
//
///**
//@function: showMainMenu
//
//@param: none
//
//@return: void
//**/
//void showMainMenu();
//
//
//
//void showLoginMenu();
//
//
//
//void showInboxMenu();
//
//
//
//void showGroupMenu();
//
//
//
//
//void navigateScreen(SCREEN screen);
//
//
//
//void showOneGroupMenu(const char* groupDisplayName);
//
//
//
//
