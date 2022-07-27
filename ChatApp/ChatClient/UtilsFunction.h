#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>
#include <iostream>

/**
*	@function: isNumeric - check if char array contains only digits ?
*	@param s(char *) - a pointer to a input string
*
*	@return 1 if string s only contains digits (0 if not)
**/
int isNumeric(char *s);


/**
*	@function: simple_tokenizer - seperate string by whitespace to vector of words
*	@param s(string) - a string
*
*	@return a vector of words seperated from s
**/
vector<string> simple_tokenizer(string s);


/**
*	@function: Receive - wrapped function recv()
*
*	@param s(SOCKET) - a socket to receive data from
*	@param request(string) - buff request
*	@param size(int) - size of buff
*	@param flags(int) - flag
*
*	@return a vector of words seperated from s
**/
int Receive(SOCKET s, char *buff, int size, int flags);


/**
*	@function: Send - wrapped function send()
*
*	@param s(SOCKET) - a socket to send data to
*	@param request(string) - buff request
*	@param size(int) - size of buff
*	@param flags(int) - flag
*
*	@return a vector of words seperated from s
**/
int Send(SOCKET s, string request, int size, int flags);


/**
*	@function: 3 overloading function to send request to server
*	@param clientSock (SOCKET) : socket to send data to 
*
*	@return
**/
void sendProcessing(SOCKET clientSock, string header);



void sendProcessing(SOCKET clientSock, string header, string s1);



void sendProcessing(SOCKET clientSock, string header, string s1, string s2);


/**
*	@function: getChoice - get option in number from users
*	@param maxChoice(int): max numbers of choice can handle
*
*	@return user's choice
**/
int getChoice(int maxChoice);


/**
*	@function: showLoginMenu - show options in Login Menu
*	@param 
*
*	@return
**/
void showLoginMenu();


/**
*	@function: showMainMenu - show options in Main Menu
*	@param
*
*	@return
**/
void showMainMenu();


/**
*	@function: showChatMenu - show options in Chat Menu
*	@param
*
*	@return
**/
void showChatMenu();


/**
*	@function: showInboxMenu - show options in Inbox Menu
*	@param userChatWith (string) - username to chat with
*
*	@return
**/
void showInboxMenu(string userChatWith);


/**
*	@function: showGroupMenu - show options in Group Menu
*	@param
*
*	@return
**/
void showGroupMenu();


/**
*	@function: showMessageHistory - show messages with other users
*	@param userChatWith (string) - username to chat with
*
*	@return
**/
void showMessageHistory(string userChatWith);


/**
*	@function: showOneGroupMenu - show options to handle in 1 group
*	@param groupName (string)
*
*	@return
**/
void showOneGroupMenu(const char* groupDisplayName);



