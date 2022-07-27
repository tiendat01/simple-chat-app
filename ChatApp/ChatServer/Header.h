#include <winsock2.h>
#include <WS2tcpip.h>
#include <windows.h>
#include <stdio.h>
#include <process.h>
#include <iostream>
#include <conio.h>
#include <string>
#include <list>
#include <map>
#include <utility>
#include <fstream>
#include <queue>
#include <cstring>
#include <sstream>
using namespace std;

#define PORT 5500
#define DATA_BUFSIZE 8192
#define RECEIVE 0
#define SEND 1
#define SERVER_ADDR "127.0.0.1"
#define ENDING_DELIMITER "\r\n"

#define BUFF_SIZE 2048
#define ENDING_DELIMITER "\r\n"
#define DELIMITER1 " "
#define DELIMITER2 "/"

// request message code 
#define LOGIN_REQ "LOGIN"
#define SIGNUP_REQ "REGISTER"
#define LOGOUT_REQ "LOGOUT"
#define ONLINE_REQ "LISTONLINE" 
#define CHECK_REQ "CHECK"
#define BLOCK_REQ "BLOCK"
#define UNBLOCK_REQ "UNBLOCK"
#define SEND_REQ "SEND"
#define LOAD_CONVERSATION_REQ "LOADCONVERSATION"
#define CREATE_GROUP_REQ "CREATEGROUP"
#define LEAVE_GROUP_REQ "LEAVEGROUP"
#define INVITE_REQ "INVITE"
#define GET_INVITATION_REQ "GETINVITATION"
#define ACCEPT_REQ "ACCEPT"
#define DENY_REQ "DENY"
#define LIST_JOINED_GROUP_REQ "LISTGROUP"

#define ACCOUNT_LEAVE_GROUP "LEAVE" 
#define LIST_GROUP "LISTGROUP"
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




// Structure definition
typedef struct {
	WSAOVERLAPPED overlapped;
	WSABUF dataBuff;
	CHAR buffer[DATA_BUFSIZE];
	int recvBytes;
	int sentBytes;
	int operation;
} PER_IO_OPERATION_DATA, *LPPER_IO_OPERATION_DATA;

// UserSession
typedef struct {
	SOCKET socket;
	SOCKADDR_IN clientAddr;
	int status; // 0 or 1 for login
	string username;
} PER_HANDLE_DATA, *LPPER_HANDLE_DATA;

typedef struct {
	HANDLE completionPort;
	SOCKET listenSock;
	SOCKADDR_IN serverAddr;
	list<SOCKET> clientSockList;
} SOCKET_IOCP, *LPSOCKET_IOCP;
