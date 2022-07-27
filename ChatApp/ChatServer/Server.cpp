// SingleIOCPServer.cpp : Defines the entry point for the console application.
//

//NOTE:
//1. Block ban than
//2. Linh sua lai ten mot so thong diep
//3. Demo > 30 messages
//4. Your choice 2 lan


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
#include "Header.h"
using namespace std;


#pragma comment(lib, "Ws2_32.lib")




map<string, tuple<string, SOCKET, int>> users;
map<int, string> groups;
map<string, vector<string>> block;



int autoIncrementGroupId = 1; // groupId start from 1 and auto increment

unsigned __stdcall serverWorkerThread(LPVOID CompletionPortID);

void readUniqueGroupId() {
	ifstream file(".\\database\\unique-group-id.txt");
	string line;
	if (!file.is_open()) {
		cerr << "Could not open file " << "\\database\\account.txt" << endl;
		exit(1);
	}
	file >> line;
	autoIncrementGroupId = atoi(line.c_str());
	file.close();
}

void readAccountDb() {
	ifstream file(".\\database\\account.txt");
	string line;
	if (!file.is_open()) {
		cerr << "Could not open file " << "\\database\\account.txt" << endl;
		exit(1);
	}
	users.clear();
	while (getline(file, line)) {
		string delimeter = " ";
		string username = line.substr(0, line.find(delimeter));
		string password = line.substr(line.find(delimeter) + 1);
		cout << username << " " << password << endl;
		users.insert({ username, make_tuple(password, 0, 0) });
	}
	file.close();
}

void readGroupDb() {
	ifstream file(".\\database\\group.txt");
	string line;
	if (!file.is_open()) {
		cerr << "Could not open file " << ".\\database\\group.txt" << endl;
		exit(1);
	}
	groups.clear();
	while (getline(file, line)) {
		string groupIdStr = line.substr(0, line.find(" "));
		string groupDisplayName = line.substr(line.find(" ") + 1);
		int groupId = atoi(groupIdStr.c_str());
		groups.insert({ groupId, groupDisplayName });
	}
	file.close();
}

void readBlockDb() {
	ifstream file(".\\database\\block.txt");
	string line;
	if (!file.is_open()) {
		cerr << "Could not open file " << ".\\database\\block.txt" << endl;
		exit(1);
	}
	block.clear();
	while (getline(file, line)) {
		string delimeter = " ";
		string blocker = line.substr(0, line.find(delimeter));
		string blocked = line.substr(line.find(delimeter) + 1);
		block[blocker].push_back(blocked);
	}
	file.close();
}


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



/// ########### PART 1: LOG IN, SIGN UP, LOG OUT ###########

string handleLoginRequest(string content, LPPER_HANDLE_DATA client) {
	string username = content.substr(0, content.find(" "));
	string password = content.substr(content.find(" ") + 1);

	if (username.length() <= 0)
		return string(BAD_REQUEST);

	if (users.find(username) == users.end())
		return string(ACCOUNT_NOT_FOUND);

	auto info = users.at(username); // info is a tuple

	if (get<0>(info) != password)
		return string(INCORRECT_PASSWORD);

	// already logged in
	if (get<2>(info) == 1) {
		if (client->socket != get<1>(info))
			return string(ACCOUNT_OTHER_SESSION);
		return string(ACCOUNT_LOGGED_IN);
	}

	// login successfully
	client->status = 1;
	//client->username = username;
	client->username.clear();
	for (int i = 0; i < username.size(); i++) client->username += username[i];
	client->username[0] = username[0];

	map<string, tuple<string, SOCKET, int>>::iterator it = users.find(username);

	if (it != users.end()) {
		it->second = make_tuple(password, client->socket, client->status);
	}

	return string(LOGIN_SUCCESS) + " " + username;

}

string handleSignupRequest(string content) {
	string username = content.substr(0, content.find(" "));
	string password = content.substr(content.find(" ") + 1);

	if (username.length() <= 0)
		return string(BAD_REQUEST);

	if (users.find(username) != users.end()) {
		return string(ACCOUNT_EXISTED);
	}
	else {
		// update to database
		ofstream file(".\\database\\account.txt", std::ios_base::app);
		if (!file.is_open()) {
			cerr << "Could not open file " << "\\database\\account.txt" << endl;
			exit(1);
		}
		file << username << " " << password << endl;
		readAccountDb(); // reload data
		file.close();

		string filename = ".\\database\\joined-group-by-username\\" + username + ".txt";
		file.open(filename, std::ios_base::app | std::ios_base::out);
		file.close();

		filename = ".\\database\\invite-request\\" + username + ".txt";
		file.open(filename, std::ios_base::app | std::ios_base::out);
		file.close();

		return string(SIGNUP_SUCCESS);
	}
}

string handleLogoutRequest(LPPER_HANDLE_DATA client) {

	if (users.find(client->username) != users.end()) {
		auto tuple1 = users.at(client->username);
		if (get<2>(tuple1) == 0)
			return string(ACCOUNT_NOT_LOGGED_IN);
		else {

			map<string, tuple<string, SOCKET, int>>::iterator it = users.find(client->username);

			if (it != users.end()) {
				it->second = make_tuple(get<0>(tuple1), client->socket, 0);
			}
			client->status = 0;
			client->username = "";

			return string(LOGOUT_SUCESS);

		}
	}
	return string(BAD_REQUEST);
}


/// ########### PART 2: SINGLE CHAT ###########


string handleListOnlineRequest() {
	string listOnline;
	for (auto& user : users) {
		tuple<string, SOCKET, int> userInfo = user.second;
		if (get<2>(userInfo) == 1) listOnline += " " + user.first;
	}
	return string(LIST_ONLINE) + listOnline;
}

string handleCheckUserRequest(string username) {
	for (auto& user : users) {
		if (user.first == username) return string(USERNAME_EXIST);
	}
	return string(USERNAME_NOT_EXIST);
}


string handleSendMessageRequest(string request, LPPER_HANDLE_DATA client) {
	string sender = client->username;
	string receiver = request.substr(0, request.find(" "));
	string message = request.substr(request.find(" ") + 1);

	for (string user : block[sender]) {
		if (user == receiver) {
			return string(BLOCK_BEFORE);
		}
	}

	for (string user : block[receiver]) {
		if (user == sender) {
			return string(BLOCKED_BEFORE);
		}
	}
	string path1 = ".\\database\\messages\\" + sender + "-" + receiver + ".txt";
	string path2 = ".\\database\\messages\\" + receiver + "-" + sender + ".txt";
	ofstream file1(path1, std::ios_base::app);
	ofstream file2(path2, std::ios_base::app);
	file1 << sender << ": " << message << "\n";
	file2 << sender << ": " << message << "\n";
	file1.close();
	file2.close();

	//check number of lines

	ifstream file(path1);
	string line;
	int numberOfLines = 0;
	while (getline(file, line)) numberOfLines++;
	file.close();
	if (numberOfLines > 30) {
		ifstream file(path1);
		ofstream newfile1("temp1.txt", std::ios_base::app);
		ofstream newfile2("temp2.txt", std::ios_base::app);
		getline(file, line); //delete the first line
		while (getline(file, line)) {
			newfile1 << line << "\n";
			newfile2 << line << "\n";
		}
		file.close(); newfile1.close(); newfile2.close();
		const char* p1 = path1.c_str();
		const char* p2 = path2.c_str();
		remove(p1); remove(p2);
		rename("temp1.txt", p1);
		rename("temp2.txt", p2);
	}

	//Send a message to receive client
	tuple<string, SOCKET, int>receiveClient = users[receiver];
	string sendMessage = RECEICE_MESSAGE + string(DELIMITER1) + sender + string(DELIMITER1) + sender + ": " + message + ENDING_DELIMITER;
	cout << sendMessage;
	send(get<1>(receiveClient), sendMessage.c_str(), sendMessage.size(), 0);

	return string(SEND_SUCCESS);

}


string handleLoadMessagesRequest(string request, LPPER_HANDLE_DATA client) {
	string sender = client->username;
	string receiver = request.substr(0, request.find(" "));
	string allMessages;

	ifstream file(".\\database\\messages\\" + sender + "-" + receiver + ".txt");
	if (!file.is_open()) {
		cerr << "Could not open file " << ".\\database\\messages\\" + sender + "-" + receiver + ".txt" << endl;
		exit(1);
	}
	string line;
	while (getline(file, line)) {
		allMessages += line + "\n";
	}

	return string(CONVERSATION_HISTORY) + " " + allMessages;

}

string handleBlockRequest(string blocked, LPPER_HANDLE_DATA client) {
	string blocker = client->username;
	for (string user : block[blocker]) {
		if (user == blocked) {
			return string(BLOCK_BEFORE);
		}
	}
	for (string user : block[blocked]) {
		if (user == blocker) {
			return string(BLOCKED_BEFORE);
		}
	}
	// update to database
	ofstream file(".\\database\\block.txt", std::ios_base::app);
	if (!file.is_open()) {
		cerr << "Could not open file " << "\\database\\block.txt" << endl;
		exit(1);
	}
	file << blocker << " " << blocked << endl;
	readBlockDb(); // reload data

	file.close();



	return string(BLOCK_SUCCESS);
}

string handleUnblockRequest(string blocked, LPPER_HANDLE_DATA client) {
	string blocker = client->username;

	for (string user : block[blocker]) {
		if (user == blocked) {
			string deleteLine = blocker + " " + blocked;
			string blockPath = ".\\database\\block.txt", tempPath = ".\\database\\temp.txt";
			ifstream file(blockPath, std::ios_base::app);
			if (!file.is_open()) {
				cerr << "Could not open file " << "\\database\\block.txt" << endl;
				exit(1);
			}
			ofstream temp(tempPath, std::ios_base::app);
			string line;
			while (getline(file, line)) {
				if (line != deleteLine) {
					temp << line << "\n";
				}
			}
			temp.close(); file.close();
			const char* p = blockPath.c_str();
			remove(p);
			rename(tempPath.c_str(), p);

			readBlockDb();

			return string(UNBLOCK_SUCCESS);
		}
	}

	for (string user : block[blocked]) {
		if (user == blocker) {
			return string(BLOCKED_BEFORE);
		}
	}

	return string(NOT_BLOCK_BEFORE);
}


/// ########### PART 3: GROUP CHAT ###########


string handleCreateGroupRequest(string request, LPPER_HANDLE_DATA client) {

	if (request.find("/") != string::npos)
		return string(FOBBIDEN_TOKEN);
	if (client->status == 0)
		return string(ACCOUNT_NOT_LOGGED_IN);

	int groupId = autoIncrementGroupId++;
	string groupName = request;

	ofstream file;
	string filename = ".\\database\\group.txt";
	file.open(filename, std::ios_base::app | std::ios_base::out);
	file << groupId << " " << groupName << endl;
	file.close();

	// update map
	readGroupDb();

	filename = ".\\database\\group-members\\group" + to_string(groupId) + ".txt";
	file.open(filename, std::ios_base::app | std::ios_base::out);
	file << client->username << endl;
	file.close();

	filename = ".\\database\\joined-group-by-username\\" + client->username + ".txt";
	file.open(filename, std::ios_base::app | std::ios_base::out);
	file << to_string(groupId) << endl;
	file.close();

	filename = ".\\database\\group-messages\\group" + to_string(groupId) + ".txt";
	file.open(filename, std::ios_base::app | std::ios_base::out);
	file.close();

	filename = ".\\database\\unique-group-id.txt";
	file.open(filename, std::ios_base::out);
	file << autoIncrementGroupId;
	file.close();

	return string(CREATE_GROUP_SUCCESS) + " " + groupName;

}


string handleInviteRequest(string content, LPPER_HANDLE_DATA client) {
	if (client->status == 0)
		return string(ACCOUNT_NOT_LOGGED_IN);
	string destUsername = content.substr(0, content.find(" "));
	int groupId = atoi(content.substr(content.find(" ") + 1).c_str());

	cout << destUsername << " " << groupId;

	// check exception here ...
	if (users.find(destUsername) == users.end())
		return string(ACCOUNT_NOT_FOUND);

	if (groups.find(groupId) == groups.end())
		return GROUP_NOT_EXIST;

	string path = ".\\database\\group-members\\group" + to_string(groupId) + ".txt";
	string line;
	ifstream fin;
	fin.open(path, std::ios_base::in);
	if (!fin.is_open()) {
		cerr << "Could not open file " << path << endl;
		exit(1);
	}

	bool isSrcInGroup = false; // check if sender is in group
	bool isDesInGroup = false; // check if receiver has already in group

	while (getline(fin, line)) {
		if (line.find(destUsername) != string::npos) {
			isDesInGroup = true;
		}

		if (line.find(client->username) != string::npos) {
			isSrcInGroup = true;
		}
	}
	fin.close();

	if (!isSrcInGroup)
		return NOT_IN_GROUP; // sender not in group so cannot invite anyone
	if (isDesInGroup)
		return USER_ALREADY_IN_GROUP; // receiver already in group

	path = ".\\database\\invite-request\\" + destUsername + ".txt";
	fin.open(path, std::ios_base::in);
	while (getline(fin, line)) {
		if (atoi(line.substr(0, line.find(" ")).c_str()) == groupId
			&& line.substr(line.find(" ") + 1) == client->username)
			return DUPLICATE_INVITATION;
	}
	fin.close();

	// sucess
	ofstream fout;
	fout.open(path, std::ios_base::out | std::ios_base::app);
	fout << groupId << " " << client->username << endl;
	fout.close();
	return INVITE_SUCCESS;

}


string handleGetInvitation(LPPER_HANDLE_DATA client) {
	if (client->status == 0)
		return string(ACCOUNT_NOT_LOGGED_IN);

	string path = ".\\database\\invite-request\\" + client->username + ".txt";
	ifstream fin(path);
	string line;

	string respondBody = string(GET_INVITATION_SUCESS) + " ";
	while (getline(fin, line)) {
		int find = line.find(" ");
		int groupId = atoi(line.substr(0, find).c_str());
		string groupDisplayName = groups.at(groupId);
		string fromUsername = line.substr(find + 1);

		respondBody += to_string(groupId) + " " + groupDisplayName + " " + fromUsername + "/";
	}

	return respondBody;
}


string handleAcceptInvitation(string content, LPPER_HANDLE_DATA client) {
	if (client->status == 0)
		return string(ACCOUNT_NOT_LOGGED_IN);
	int groupId = atoi(content.c_str()); // content is groupId in string
	if (groups.find(groupId) == groups.end())
		return GROUP_NOT_EXIST;
	string 	path = ".\\database\\group-members\\group" + content + ".txt";
	ifstream fin(path);
	string line;
	vector<string> fileContent;
	while (getline(fin, line)) {
		if (line == client->username)
			return ALREADY_IN_THIS_GROUP;
	}
	fin.close();

	path = ".\\database\\invite-request\\" + client->username + ".txt";
	fin.open(path, std::ios_base::in);
	bool isInvited = false;
	while (getline(fin, line)) {
		fileContent.push_back(line);
		if (content == line.substr(0, line.find(" ")))
			isInvited = true;
	}
	fin.close();
	if (!isInvited)
		return NOT_INVITED_TO_THIS_GROUP;

	// sucess:
	path = ".\\database\\invite-request\\" + client->username + ".txt";
	ofstream fout;
	fout.open(path, std::ios_base::out | std::ios_base::trunc);
	for (auto it = fileContent.begin(); it != fileContent.end(); it++) {
		string line = *it;
		if (line.find(content) != 0)
			fout << line << endl;
	}
	fout.close();

	path = ".\\database\\group-members\\group" + content + ".txt";
	fout.open(path, std::ios_base::out | std::ios_base::app);
	fout << client->username << endl;
	fout.close();

	path = ".\\database\\joined-group-by-username\\" + client->username + ".txt";
	fout.open(path, std::ios_base::out | std::ios_base::app);
	fout << content << endl;
	fout.close();

	return ACCEPT_SUCCESS + string(DELIMITER1) + content + " " + groups.at(groupId);

}


string handleDenyInvitation(string content, LPPER_HANDLE_DATA client) {
	if (client->status == 0)
		return string(ACCOUNT_NOT_LOGGED_IN);
	int groupId = atoi(content.c_str()); // content is groupId in string
	if (groups.find(groupId) == groups.end())
		return GROUP_NOT_EXIST;
	string 	path = ".\\database\\group-members\\group" + content + ".txt";
	ifstream fin(path);
	string line;
	vector<string> fileContent;
	while (getline(fin, line)) {
		if (line == client->username)
			return ALREADY_IN_THIS_GROUP;
	}
	fin.close();

	path = ".\\database\\invite-request\\" + client->username + ".txt";
	fin.open(path, std::ios_base::in);
	bool isInvited = false;
	while (getline(fin, line)) {
		fileContent.push_back(line);
		if (content == line.substr(0, line.find(" ")))
			isInvited = true;
	}
	fin.close();
	if (!isInvited)
		return NOT_INVITED_TO_THIS_GROUP;

	// sucess:
	path = ".\\database\\invite-request\\" + client->username + ".txt";
	ofstream fout;
	fout.open(path, std::ios_base::out | std::ios_base::trunc);
	for (auto it = fileContent.begin(); it != fileContent.end(); it++) {
		string line = *it;
		if (line.find(content) != 0)
			fout << line << endl;
	}
	fout.close();

	return DENY_SUCCESS + string(DELIMITER1) + content + " " + groups.at(groupId);

}


string handleListMemberGroupRequest(string content, LPPER_HANDLE_DATA client) {
	for (int i = 0; i < content.length(); i++) {
		if (content[i] < '0' || content[i] > '9') {
			return BAD_REQUEST;
		}
	}
	int idGroupUser = atoi(content.c_str());
	string username = client->username;
	if (client->status == 0) {
		return ACCOUNT_NOT_LOGGED_IN;
	}

	if (groups.find(idGroupUser) == groups.end()) {
		return GROUP_NOT_EXIST;
	}

	string path = ".\\database\\group-members\\group" + content + ".txt";
	ifstream fin;
	fin.open(path, std::ios_base::in);
	if (!fin.is_open()) {
		cout << "Cannot open the file: " << path << endl;
		return BAD_REQUEST;
	}

	vector<string>listMembersPerGroup;
	string line;
	while (getline(fin, line)) {
		string memberUsername = line;
		listMembersPerGroup.push_back(memberUsername);
	}
	fin.close();

	string responseBody = LISTMEMBERS_SUCCESS;
	bool isInGroup = false;
	for (auto it = listMembersPerGroup.begin(); it != listMembersPerGroup.end(); it++) {
		if (*it == client->username)
			isInGroup = true;
		responseBody += " " + *it;
	}

	if (!isInGroup)
		return NOT_IN_GROUP;
	return responseBody;


}



string handleLoadMessageRequest(string content, LPPER_HANDLE_DATA client) {
	for (int i = 0; i < content.length(); i++) {
		if (content[i] < '0' || content[i] > '9') {
			return BAD_REQUEST;
		}
	}
	int idGroupUser = atoi(content.c_str());
	string username = client->username;
	if (client->status == 0) {
		return ACCOUNT_NOT_LOGGED_IN;
	}
	if (groups.find(idGroupUser) == groups.end()) {
		return GROUP_NOT_EXIST;
	}

	string path = ".\\database\\group-members\\group" + content + ".txt";
	ifstream fin;
	fin.open(path, std::ios_base::in);
	if (!fin.is_open()) {
		cout << "Cannot open the file: " << path << endl;
		return BAD_REQUEST;
	}

	vector<string>listMembersPerGroup;
	string line;
	while (getline(fin, line)) {
		string memberUsername = line;
		listMembersPerGroup.push_back(memberUsername);
	}
	fin.close();

	bool isInGroup = false;

	for (auto it = listMembersPerGroup.begin(); it != listMembersPerGroup.end(); it++) {
		if (*it == client->username) {
			isInGroup = true;
			break;
		}
	}

	if (!isInGroup)
		return NOT_IN_GROUP;

	// if success
	string responseBody = LOADMSG_SUCCESS + string(DELIMITER1) + to_string(idGroupUser) + " ";

	string history;
	path = ".\\database\\group-messages\\group" + content + ".txt";
	fin.open(path, std::ios_base::in);
	if (!fin.is_open()) {
		cout << "Cannot open file " << path << endl;
		return string(BAD_REQUEST);
	}
	while (getline(fin, line)) {
		history += line + "\n";
	}
	fin.close();
	responseBody.append(history);

	return responseBody;


}



string handleSendGroupMessageRequest(string content, LPPER_HANDLE_DATA client) {
	string sGroup = content.substr(0, content.find(" ")); // sGroup is groupId to chat in string

	for (int i = 0; i < sGroup.length(); i++) {
		if (sGroup[i] < '0' || sGroup[i] > '9') {
			return BAD_REQUEST;
		}
	}

	int idGroupUser = atoi(sGroup.c_str());
	string msg = content.substr(content.find(" ") + 1); // get message from user to chat in group
	string username = client->username;
	if (client->status == 0) {
		return ACCOUNT_NOT_LOGGED_IN;
	}
	if (groups.find(idGroupUser) == groups.end()) {
		return GROUP_NOT_EXIST;
	}

	string path = ".\\database\\group-members\\group" + sGroup + ".txt";
	ifstream fin;
	fin.open(path, std::ios_base::in);
	if (!fin.is_open()) {
		cout << "Cannot open the file: " << path << endl;
		return BAD_REQUEST;
	}

	vector<string>listMembersPerGroup;
	string line;
	while (getline(fin, line)) {
		string memberUsername = line;
		listMembersPerGroup.push_back(memberUsername);
	}
	fin.close();
	string responseBody = GROUPMESSAGE_SUCCESS + string(DELIMITER1) + to_string(idGroupUser) + " " + username;
	bool isInGroup = false;
	for (auto it = listMembersPerGroup.begin(); it != listMembersPerGroup.end(); it++) {
		if (*it == client->username) {
			isInGroup = true;
			break;
		}
	}

	if (!isInGroup)
		return NOT_IN_GROUP;


	string oneMsg = username + ": " + msg;
	path = ".\\database\\group-messages\\group" + sGroup + ".txt";
	ofstream fout;
	fout.open(path, std::ios_base::out | std::ios_base::app);
	if (!fout.is_open()) {
		cout << "Cannot open file " << path << endl;
		return BAD_REQUEST;
	}
	fout << oneMsg << endl;
	fout.close();
	responseBody += " " + oneMsg;
	return responseBody;
}


string handleListGroupRequest(LPPER_HANDLE_DATA client) {
	if (client->status == 0)
		return string(ACCOUNT_NOT_LOGGED_IN);

	string path = ".\\database\\joined-group-by-username\\" + client->username + ".txt";
	ifstream file(path);
	string line;
	if (!file.is_open()) {
		cerr << "Could not open file " << path << endl;
		return string(LIST_GROUP_SUCCESS) + "/";
	}

	string responseBody = string(LIST_GROUP_SUCCESS) + " ";

	while (getline(file, line)) {
		int groupId = atoi(line.c_str());
		if (groups.find(groupId) == groups.end())
			continue;
		responseBody += (line + " " + groups.at(groupId) + "/");

	}

	return responseBody;
}



string handleLeaveGroup(string content, LPPER_HANDLE_DATA client) {
	if (client->status == 0)
		return string(ACCOUNT_NOT_LOGGED_IN);

	int groupId = atoi(content.c_str());
	cout << groupId;

	if (groups.find(groupId) == groups.end())
		return GROUP_NOT_EXIST;

	vector<string> contentFile1, contentFile2;

	string path = ".\\database\\group-members\\group" + to_string(groupId) + ".txt";
	string line;
	ifstream fin;
	fin.open(path, std::ios_base::in);
	if (!fin.is_open()) {
		cerr << "Could not open file " << path << endl;
		return BAD_REQUEST; // this should be another code
	}

	bool isSrcInGroup = false; // check if sender is in group
	while (getline(fin, line)) {
		contentFile1.push_back(line);
		if (line.find(client->username) != string::npos) {
			isSrcInGroup = true;
		}
	}
	fin.close();

	path = ".\\database\\joined-group-by-username\\" + client->username + ".txt";
	fin.open(path, std::ios_base::in);
	while (getline(fin, line))
		contentFile2.push_back(line);
	fin.close();


	if (!isSrcInGroup)
		return NOT_IN_GROUP; // sender not in group so cannot leave this group

					  // successfully leave group
	path = ".\\database\\group-members\\group" + to_string(groupId) + ".txt";
	ofstream fout;
	fout.open(path, std::ios_base::out | std::ios_base::trunc);
	for (auto it = contentFile1.begin(); it != contentFile1.end(); it++)
		if (*it != client->username)
			fout << *it << endl;
	fout.close();

	path = ".\\database\\joined-group-by-username\\" + client->username + ".txt";
	fout.open(path, std::ios_base::out | std::ios_base::trunc);
	for (auto it = contentFile2.begin(); it != contentFile2.end(); it++)
		if (*it != content)
			fout << *it << endl;
	fout.close();
	return LEAVE_GROUP_SUCCESS;
}






/// ###### FILTER REQUEST AND NAVIGATION TO SUITABLE FUNCTION ##################


string outputResponseFrom(string request, LPPER_HANDLE_DATA client) {

	/// PART 1

	// LOGIN
	if (request.find(LOGIN_REQ) != string::npos && request.find(LOGIN_REQ) == 0
		&& request.find(" ") == strlen(LOGIN_REQ)) {
		request = request.substr(strlen(LOGIN_REQ) + 1);
		return handleLoginRequest(request, client);
	}

	// SIGNUP
	if (request.find(SIGNUP_REQ) != string::npos && request.find(SIGNUP_REQ) == 0
		&& request.find(" ") == strlen(SIGNUP_REQ)) {
		request = request.substr(strlen(SIGNUP_REQ) + 1);
		return handleSignupRequest(request);
	}

	// LOGOUT
	if (request.find(LOGOUT_REQ) != string::npos && request.find(LOGOUT_REQ) == 0) {
		return handleLogoutRequest(client);
	}

	if (request == ONLINE_REQ) {
		return handleListOnlineRequest();
	}

	if (request.find(CHECK_REQ) != string::npos && request.find(CHECK_REQ) == 0
		&& request.find(" ") == strlen(CHECK_REQ)) {
		request = request.substr(strlen(CHECK_REQ) + 1);
		return handleCheckUserRequest(request);
	}

	if (request.find(SEND_REQ) != string::npos && request.find(SEND_REQ) == 0
		&& request.find(" ") == strlen(SEND_REQ)) {
		request = request.substr(strlen(SEND_REQ) + 1);
		return handleSendMessageRequest(request, client);
	}

	if (request.find(LOAD_CONVERSATION_REQ) != string::npos && request.find(LOAD_CONVERSATION_REQ) == 0
		&& request.find(" ") == strlen(LOAD_CONVERSATION_REQ)) {
		request = request.substr(strlen(LOAD_CONVERSATION_REQ) + 1);
		return handleLoadMessagesRequest(request, client);
	}

	if (request.find(BLOCK_REQ) != string::npos && request.find(BLOCK_REQ) == 0
		&& request.find(" ") == strlen(BLOCK_REQ)) {
		request = request.substr(strlen(BLOCK_REQ) + 1);
		return handleBlockRequest(request, client);
	}

	if (request.find(UNBLOCK_REQ) != string::npos && request.find(UNBLOCK_REQ) == 0
		&& request.find(" ") == strlen(UNBLOCK_REQ)) {
		request = request.substr(strlen(UNBLOCK_REQ) + 1);
		return handleUnblockRequest(request, client);
	}


	// CREATE NEW GROUP: CREATEGROUP ... (if not => bad request)
	if (request.find(CREATE_GROUP_REQ) != string::npos && request.find(CREATE_GROUP_REQ) == 0
		&& request.find(" ") == strlen(CREATE_GROUP_REQ)) {
		request = request.substr(strlen(CREATE_GROUP_REQ) + 1);
		return handleCreateGroupRequest(request, client);
	}


	/// PART 3

	// INVITE TO NEW GROUP: INVITE username groupId (if not => bad request)
	if (request.find(INVITE_REQ) != string::npos && request.find(INVITE_REQ) == 0
		&& request.find(" ") == strlen(INVITE_REQ)) {
		request = request.substr(strlen(INVITE_REQ) + 1);
		if (request.find(" ") != string::npos)
			return handleInviteRequest(request, client);
	}


	// GET INVITATION
	if (request.find(GET_INVITATION_REQ) != string::npos && request.find(GET_INVITATION_REQ) == 0) {
		return handleGetInvitation(client);
	}


	// ACCEPT REQ
	if (request.find(ACCEPT_REQ) != string::npos && request.find(ACCEPT_REQ) == 0
		&& request.find(" ") == strlen(ACCEPT_REQ)) {
		request = request.substr(strlen(ACCEPT_REQ) + 1);
		return handleAcceptInvitation(request, client);
	}

	// DENY REQ
	if (request.find(DENY_REQ) != string::npos && request.find(DENY_REQ) == 0
		&& request.find(" ") == strlen(DENY_REQ)) {
		request = request.substr(strlen(DENY_REQ) + 1);
		return handleDenyInvitation(request, client);
	}


	// LIST JOINED GROUP of client
	if (request.find(LIST_JOINED_GROUP_REQ) != string::npos && request.find(LIST_JOINED_GROUP_REQ) == 0) {
		return handleListGroupRequest(client);
	}

	// LEAVE GROUP
	if (request.find(LEAVE_GROUP_REQ) != string::npos && request.find(LEAVE_GROUP_REQ) == 0
		&& request.find(" ") == strlen(LEAVE_GROUP_REQ)) {
		request = request.substr(strlen(LEAVE_GROUP_REQ) + 1);
		return handleLeaveGroup(request, client);
	}


	// LIST MEMBERS IN A GROUP
	if (request.find(LIST_MEMBERS_GROUP) == 0) {//linh
		request = request.substr(string(LIST_MEMBERS_GROUP).length() + 1);
		return handleListMemberGroupRequest(request, client);
	}


	// RETURN ALL HISTORY CHAT IN GROUP
	if (request.find(LOAD_GROUP_MESSAGE) == 0) {//linh
		request = request.substr(string(LOAD_GROUP_MESSAGE).length() + 1);
		return handleLoadMessageRequest(request, client);
	}


	// HANDLE WHEN AN USER SEND NEW MESSAGE
	if (request.find(GROUP_MESSAGE) == 0) {//linh
		request = request.substr(string(GROUP_MESSAGE).length() + 1);
		return handleSendGroupMessageRequest(request, client);
	}




	return string(BAD_REQUEST);
}




int main(int argc, CHAR* argv[])
{
	SOCKET acceptSock;

	SOCKADDR_IN clientAddr;

	LPSOCKET_IOCP iocp = new SOCKET_IOCP(); // server socket info and IOCP

	SYSTEM_INFO systemInfo;

	LPPER_HANDLE_DATA perHandleData;
	LPPER_IO_OPERATION_DATA perIoData;

	DWORD transferredBytes;
	DWORD flags;

	WSADATA wsaData;

	// Step 0: read file
	readAccountDb();
	readBlockDb();
	readGroupDb();
	readUniqueGroupId();


	// Step 1: Init Winsock
	if (WSAStartup((2, 2), &wsaData) != 0) {
		printf("WSAStartup() failed with error %d\n", GetLastError());
		return 1;
	}

	// Step 2: Create an I/O completion port
	if ((iocp->completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0))
		== NULL) {
		printf("CreateIoCompletionPort() failed with error %d\n", GetLastError());
		return 1;
	}

	// Step 3: Determine how many processors are on the system
	GetSystemInfo(&systemInfo);

	// Step 4: Create worker threads based on the number of processors available on the
	// system. Create two worker threads for each processor	
	for (int i = 0; i < (int)systemInfo.dwNumberOfProcessors * 2; i++) {
		// Create a server worker thread and pass the completion port to the thread
		if (_beginthreadex(0, 0, serverWorkerThread, (void*)iocp, 0, 0) == 0) {
			printf("Create thread failed with error %d\n", GetLastError());
			return 1;
		}
	}

	// Step 5: Create a listening socket
	if ((iocp->listenSock = WSASocketW(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED))
		== INVALID_SOCKET) {
		printf("WSASocket() failed with error %d\n", WSAGetLastError());
		return 1;
	}

	// bind
	iocp->serverAddr.sin_family = AF_INET;
	iocp->serverAddr.sin_port = htons(PORT);
	inet_pton(AF_INET, SERVER_ADDR, &iocp->serverAddr.sin_addr);
	if (bind(iocp->listenSock, (PSOCKADDR)&iocp->serverAddr, sizeof(iocp->serverAddr)) == SOCKET_ERROR) {
		printf("bind() failed with error %d\n", WSAGetLastError());
		return 1;
	}

	// Prepare socket for listening
	if (listen(iocp->listenSock, 10) == SOCKET_ERROR) {
		printf("listen() failed with error %d\n", WSAGetLastError());
		return 1;
	}

	cout << "Server start!\n";

	while (1) {

		// Step 6: Accept connections
		if ((acceptSock = WSAAccept(iocp->listenSock, (SOCKADDR*)&clientAddr, NULL, NULL, 0)) == SOCKET_ERROR) {
			printf("WSAAccept() failed with error %d\n", WSAGetLastError());
			return 1;
		}

		iocp->clientSockList.push_back(acceptSock);

		// Step 7: Create a socket information structure to associate with the socket
		if ((perHandleData = (LPPER_HANDLE_DATA)GlobalAlloc(GPTR, sizeof(PER_HANDLE_DATA))) == NULL) {
			printf("GlobalAlloc() failed with error %d\n", GetLastError());
			return 1;
		}
		char clientIP[INET_ADDRSTRLEN];
		cout << "Accept connection from IP Address: " + string(inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, sizeof(clientIP)))
			+ " Port: " + to_string(ntohs(clientAddr.sin_port));

		// Step 8: Associate the accepted socket with the original completion port
		printf("\nSocket number %d got connected...\n", acceptSock);
		perHandleData->socket = acceptSock; // tim 1 cho trong va dua vao mang clients: clients[i]->socket = acceptSock
		perHandleData->status = 0;
		memcpy(&(perHandleData->clientAddr), &clientAddr, sizeof(clientAddr));

		if (CreateIoCompletionPort((HANDLE)acceptSock, iocp->completionPort,
			(DWORD)perHandleData, 0) == NULL) {
			printf("CreateIoCompletionPort() failed with error %d\n", GetLastError());
			return 1;
		}

		// Step 9: Create per I/O socket information structure to associate with the WSARecv call
		if ((perIoData = (LPPER_IO_OPERATION_DATA)GlobalAlloc(GPTR,
			sizeof(PER_IO_OPERATION_DATA))) == NULL) {
			printf("GlobalAlloc() failed with error %d\n", GetLastError());
			return 1;
		}

		ZeroMemory(&(perIoData->overlapped), sizeof(OVERLAPPED));
		perIoData->sentBytes = 0;
		perIoData->recvBytes = 0;
		perIoData->dataBuff.len = DATA_BUFSIZE;
		perIoData->dataBuff.buf = perIoData->buffer;
		perIoData->operation = RECEIVE;
		flags = 0;

		if (WSARecv(perHandleData->socket, &(perIoData->dataBuff), 1, &transferredBytes, &flags, &(perIoData->overlapped), NULL) == SOCKET_ERROR) {
			if (WSAGetLastError() != ERROR_IO_PENDING) {
				printf("WSARecv() failed with error %d\n", WSAGetLastError());
				return 1;
			}
		}
	}

	// close socket:
	for (list<SOCKET>::iterator it = iocp->clientSockList.begin();
	it != iocp->clientSockList.end(); it++) {
		closesocket(*it);
	}
	iocp->clientSockList.empty();
	CloseHandle(iocp->completionPort);

	for (int i = 0; i < (int)systemInfo.dwNumberOfProcessors * 2; i++) {
		PostQueuedCompletionStatus(iocp, 0, NULL, NULL);
	}
	closesocket(iocp->listenSock);
	delete iocp;

	WSACleanup();

	_getch();
	return 0;
}


unsigned __stdcall serverWorkerThread(LPVOID iocp)
{
	LPSOCKET_IOCP lpIocp = (LPSOCKET_IOCP)iocp;

	DWORD transferredBytes;
	LPPER_HANDLE_DATA perHandleData;
	LPPER_IO_OPERATION_DATA perIoData;
	DWORD flags;

	string prevBuff;
	queue<string> requestQueue;

	while (TRUE) {
		if (GetQueuedCompletionStatus(lpIocp->completionPort, &transferredBytes,
			(LPDWORD)&perHandleData, (LPOVERLAPPED *)&perIoData, INFINITE) == 0) {
			//printf("GetQueuedCompletionStatus() failed with error %d\n", GetLastError());
			cout << "Client disconnect!" << endl;
			handleLogoutRequest(perHandleData);
			return 0;
		}
		// Check to see if an error has occurred on the socket and if so
		// then close the socket and cleanup the SOCKET_INFORMATION structure
		// associated with the socket
		if (transferredBytes == 0) {
			printf("Closing socket %d\n", perHandleData->socket);
			if (closesocket(perHandleData->socket) == SOCKET_ERROR) {
				printf("closesocket() failed with error %d\n", WSAGetLastError());
				return 0;
			}
			GlobalFree(perHandleData);
			GlobalFree(perIoData);
			// free client socket
			lpIocp->clientSockList.remove(perHandleData->socket);
			closesocket(perHandleData->socket);
			continue;
		}
		// Check to see if the operation field equals RECEIVE. If this is so, then
		// this means a WSARecv call just completed so update the recvBytes field
		// with the transferredBytes value from the completed WSARecv() call
		if (perIoData->operation == RECEIVE) {
			int found = 0;
			string receivceBuffer = string(perIoData->buffer);
			receivceBuffer = prevBuff + receivceBuffer;
			prevBuff.clear();

			// stream handle
			while ((found = receivceBuffer.find("\r\n")) != string::npos) {
				if (receivceBuffer.find("\r\n") > 0) {
					requestQueue.push(receivceBuffer.substr(0, found));
					//cout << "request from client: " << receivceBuffer.substr(0, found) << endl;
					receivceBuffer = receivceBuffer.substr(found + strlen(ENDING_DELIMITER));
				}
			}
			if (receivceBuffer.length() > 0)
				prevBuff = receivceBuffer;



			// handle request
			while (!requestQueue.empty()) {

				cout << "Handle request from client: " << requestQueue.front() << endl;


				// process
				string response = outputResponseFrom(requestQueue.front(), perHandleData);

				response += ENDING_DELIMITER; // 390 1 datnt dlfjsldffsal fsdjf slfalsd f\r\n

											  // send response to one client
				LPPER_IO_OPERATION_DATA perIoDataToSend = new PER_IO_OPERATION_DATA();
				ZeroMemory(&(perIoDataToSend->overlapped), sizeof(OVERLAPPED));
				memset(perIoDataToSend, 0, sizeof(PER_IO_OPERATION_DATA));
				memcpy(perIoDataToSend->buffer, response.c_str(), response.length());
				perIoDataToSend->dataBuff.buf = perIoDataToSend->buffer;
				perIoDataToSend->dataBuff.len = response.length();
				perIoDataToSend->operation = SEND;

				DWORD bytesToSend = 0;

				if (WSASend(perHandleData->socket, &(perIoDataToSend->dataBuff), 1,
					&bytesToSend, 0, &perIoDataToSend->overlapped, NULL) == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
					if (perHandleData != NULL) {
						closesocket(perHandleData->socket);
						lpIocp->clientSockList.remove(perHandleData->socket);
					}


				string flag = response.substr(0, 3);


				if (flag == GROUPMESSAGE_SUCCESS) {
					//// group chat broadcast 
					vector<string>p = simple_tokenizer(response);
					int gr = atoi(p[1].c_str());

					string newResponse = RECEIVE_MESSAGE_GROUP + string(DELIMITER1) + p[1] + " ";
					response = response.substr(response.find(" ") + 1);
					response = response.substr(response.find(" ") + 1);
					response = response.substr(response.find(" ") + 1);

					newResponse += response; // "381 groupId content...\r\n"


					string path = ".\\database\\group-members\\group" + to_string(gr) + ".txt";
					ifstream fin;
					fin.open(path, std::ios_base::in);
					if (!fin.is_open()) {
						cout << "Cannot open the file: " << path << endl;
					}

					vector<string>listMembersPerGroup;
					string line;
					while (getline(fin, line)) {
						string memberUsername = line;
						listMembersPerGroup.push_back(memberUsername);
					}
					fin.close();


					for (int it1 = 0; it1 < listMembersPerGroup.size(); it1++) {
						auto info = users.at(listMembersPerGroup[it1]);
						if (get<2>(info) == 1 && perHandleData->username != p[2]) {
							LPPER_IO_OPERATION_DATA perIoDataToSend = new PER_IO_OPERATION_DATA();
							ZeroMemory(&(perIoDataToSend->overlapped), sizeof(OVERLAPPED));
							memset(perIoDataToSend, 0, sizeof(PER_IO_OPERATION_DATA));
							memcpy(perIoDataToSend->buffer, newResponse.c_str(), newResponse.length());
							perIoDataToSend->dataBuff.buf = perIoDataToSend->buffer;
							perIoDataToSend->dataBuff.len = newResponse.length();
							perIoDataToSend->operation = SEND;

							DWORD bytesToSend = 0;
							if (WSASend(get<1>(info), &(perIoDataToSend->dataBuff), 1,
								&bytesToSend, 0, &perIoDataToSend->overlapped, NULL) == SOCKET_ERROR
								&& WSAGetLastError() != WSA_IO_PENDING) {
								if (perHandleData != NULL) {
									if (get<1>(info) != NULL) {
										closesocket(get<1>(info));
										lpIocp->clientSockList.remove(perHandleData->socket);
									}
								}
							}
						}
					}
				}



				requestQueue.pop();
			}


			// server continue being at state RECEIVE
			ZeroMemory(&(perIoData->overlapped), sizeof(OVERLAPPED));
			memset(perIoData, 0, sizeof(PER_IO_OPERATION_DATA));
			perIoData->dataBuff.buf = perIoData->buffer;
			perIoData->dataBuff.len = sizeof(perIoData->buffer);
			perIoData->operation = RECEIVE;
			DWORD bytesToReceive = 0, flag = 0;
			WSARecv(perHandleData->socket, &(perIoData->dataBuff), 1, &bytesToReceive,
				&flag, &(perIoData->overlapped), NULL);

			continue;
		}
		else {
			// WSASend() success
			if (perIoData != NULL) {
				delete perIoData;
				perIoData = NULL;
			}
			continue;
		}
	} // end while (1)
	return 0;
}











//// SingleIOCPServer.cpp : Defines the entry point for the console application.
////
//
//#include <winsock2.h>
//#include <WS2tcpip.h>
//#include <windows.h>
//#include <stdio.h>
//#include <process.h>
//#include <iostream>
//#include <conio.h>
//#include <string>
//#include <list>
//#include <map>
//#include <utility>
//#include <fstream>
//#include <queue>
//#include <cstring>
//#include <sstream>
//
//using namespace std;
//#pragma comment(lib, "Ws2_32.lib")
//
//
//#define PORT 5500
//#define DATA_BUFSIZE 8192
//#define RECEIVE 0
//#define SEND 1
//#define SERVER_ADDR "127.0.0.1"
//#define ENDING_DELIMITER "\r\n"
//
//// request message code 
//#define LOGIN_REQ "LOGIN"
//#define SIGNUP_REQ "REGISTER"
//#define LOGOUT_REQ "LOGOUT"
//
//#define CREATE_GROUP_REQ "CREATEGROUP"
//#define LEAVE_GROUP_REQ "LEAVEGROUP"
//
//
//#define INVITE_REQ "INVITE"
//#define GET_INVITATION_REQ "GETINVITATION"
//#define ACCEPT_REQ "ACCEPT"
//#define DENY_REQ "DENY"
//#define LIST_JOINED_GROUP_REQ "LISTGROUP"
//
//
//// response code
//#define LOGIN_SUCCESS "100"
//#define ACCOUNT_NOT_FOUND "101"
//#define INCORRECT_PASSWORD "102"
//#define ACCOUNT_LOGGED_IN "103"
//#define ACCOUNT_OTHER_SESSION "104"
//
//#define SIGNUP_SUCCESS "110"
//#define ACCOUNT_EXISTED "111"
//
//#define LOGOUT_SUCESS "120"
//
//#define CREATE_GROUP_SUCCESS "300"
//#define FOBBIDEN_TOKEN "301"
//
//#define LIST_GROUP_SUCCESS "360"
//
//
//#define ACCOUNT_LEAVE_GROUP "LEAVE" 
//#define LIST_GROUP "LISTGROUP"
//#define LIST_MEMBERS_GROUP "LISTMEMBERS" 
//#define LOAD_MESSAGE  "LOADMESSAGE"
//#define MESSAGE "MESSAGE"
//
//#define GET_INVITATION_SUCESS "320"
//
//#define BAD_REQUEST "999"
//#define ACCOUNT_NOT_LOGGED_IN "201"
//
//
//// Structure definition
//typedef struct {
//	WSAOVERLAPPED overlapped;
//	WSABUF dataBuff;
//	CHAR buffer[DATA_BUFSIZE];
//	int recvBytes;
//	int sentBytes;
//	int operation;
//} PER_IO_OPERATION_DATA, *LPPER_IO_OPERATION_DATA;
//
//// UserSession
//typedef struct {
//	SOCKET socket;
//	SOCKADDR_IN clientAddr;
//	int status; // 0 or 1 for login
//	string username;
//} PER_HANDLE_DATA, *LPPER_HANDLE_DATA;
//
//typedef struct {
//	HANDLE completionPort;
//	SOCKET listenSock;
//	SOCKADDR_IN serverAddr;
//	list<SOCKET> clientSockList;
//} SOCKET_IOCP, *LPSOCKET_IOCP;
//
//
//map<string, tuple<string, SOCKET, int>> users;
//map<int, string> groups;
//
//
//int autoIncrementGroupId = 1; // groupId start from 1 and auto increment
//
//
//unsigned __stdcall serverWorkerThread(LPVOID CompletionPortID);
//
//void readUniqueGroupId() {
//	ifstream file(".\\database\\unique-group-id.txt");
//	string line;
//	if (!file.is_open()) {
//		cerr << "Could not open file " << "\\database\\account.txt" << endl;
//		exit(1);
//	}
//	file >> line;
//	autoIncrementGroupId = atoi(line.c_str());
//	file.close();
//}
//
//void readAccountDb() {
//	ifstream file(".\\database\\account.txt");
//	string line;
//	if (!file.is_open()) {
//		cerr << "Could not open file " << "\\database\\account.txt" << endl;
//		exit(1);
//	}
//	users.clear();
//	while (getline(file, line)) {
//		string delimeter = " ";
//		string username = line.substr(0, line.find(delimeter));
//		string password = line.substr(line.find(delimeter) + 1);
//		users.insert({ username, make_tuple(password, 0, 0) });
//	}
//	file.close();
//}
//
//void readGroupDb() {
//	ifstream file(".\\database\\group.txt");
//	string line;
//	if (!file.is_open()) {
//		cerr << "Could not open file " << ".\\database\\group.txt" << endl;
//		exit(1);
//	}
//	groups.clear();
//	while (getline(file, line)) {
//		string groupIdStr = line.substr(0, line.find(" "));
//		string groupDisplayName = line.substr(line.find(" ") + 1);
//		int groupId = atoi(groupIdStr.c_str());
//		groups.insert({ groupId, groupDisplayName });
//	}
//	file.close();
//}
//
//
//
//vector<string> simple_tokenizer(string s)
//{
//	stringstream ss(s);
//	string word;
//	vector<string>sp;
//	while (ss >> word) {
//		sp.push_back(word);
//	}
//	return sp;
//}
//
//
//
///// ########### PART 1: LOG IN, SIGN UP, LOG OUT ###########
//
//string handleLoginRequest(string content, LPPER_HANDLE_DATA client) {
//	string username = content.substr(0, content.find(" "));
//	string password = content.substr(content.find(" ") + 1);
//
//	if (username.length() <= 0)
//		return string(BAD_REQUEST);
//
//	if (users.find(username) == users.end())
//		return string(ACCOUNT_NOT_FOUND);
//
//	auto info = users.at(username); // info is a tuple
//
//	if (get<0>(info) != password)
//		return string(INCORRECT_PASSWORD);
//
//	// already logged in
//	if (get<2>(info) == 1) {
//		if (client->socket != get<1>(info))
//			return string(ACCOUNT_OTHER_SESSION);
//		return string(ACCOUNT_LOGGED_IN);
//	}
//
//	// login successfully
//	client->status = 1;
//	client->username = username;
//
//	map<string, tuple<string, SOCKET, int>>::iterator it = users.find(username);
//
//	if (it != users.end()) {
//		it->second = make_tuple(password, client->socket, client->status);
//	}
//
//	return string(LOGIN_SUCCESS) + " " + username;
//
//}
//
//string handleSignupRequest(string content) {
//	string username = content.substr(0, content.find(" "));
//	string password = content.substr(content.find(" ") + 1);
//
//	if (username.length() <= 0)
//		return string(BAD_REQUEST);
//
//	if (users.find(username) != users.end()) {
//		return string(ACCOUNT_EXISTED);
//	}
//	else {
//		// update to database
//		ofstream file(".\\database\\account.txt", std::ios_base::app);
//		if (!file.is_open()) {
//			cerr << "Could not open file " << "\\database\\account.txt" << endl;
//			exit(1);
//		}
//		file << username << " " << password << endl;
//		readAccountDb(); // reload data
//		file.close();
//
//		string filename = ".\\database\\joined-group-by-username\\" + username + ".txt";
//		file.open(filename, std::ios_base::app | std::ios_base::out);
//		file.close();
//		
//		filename = ".\\database\\invite-request\\" + username + ".txt";
//		file.open(filename, std::ios_base::app | std::ios_base::out);
//		file.close();
//
//		return string(SIGNUP_SUCCESS);
//	}
//}
//
//string handleLogoutRequest(LPPER_HANDLE_DATA client) {
//
//	if (users.find(client->username) != users.end()) {
//		auto tuple1 = users.at(client->username);
//		if (get<2>(tuple1) == 0)
//			return string(ACCOUNT_NOT_LOGGED_IN);
//		else {
//
//			map<string, tuple<string, SOCKET, int>>::iterator it = users.find(client->username);
//
//			if (it != users.end()) {
//				it->second = make_tuple(get<0>(tuple1), client->socket, 0);
//			}
//			client->status = 0;
//			client->username = "";
//			
//			return string(LOGOUT_SUCESS);
//
//		}
//	}
//	return string(BAD_REQUEST);
//}
//
//
//
//
///// ########### PART 3: GROUP CHAT ###########
//
//
//string handleCreateGroupRequest(string request, LPPER_HANDLE_DATA client) {
//	
//	if (request.find("/") != string::npos)
//		return string(FOBBIDEN_TOKEN);
//	if (client->status == 0)
//		return string(ACCOUNT_NOT_LOGGED_IN);
//
//	int groupId = autoIncrementGroupId++; // from 1
//	string groupName = request;
//
//	ofstream file;
//	string filename = ".\\database\\group.txt";
//	file.open(filename, std::ios_base::app | std::ios_base::out);
//	file << groupId << " " << groupName << endl;
//	file.close();
//
//	// update map
//	readGroupDb();
//
//	filename = ".\\database\\group-members\\group" + to_string(groupId) + ".txt";
//	file.open(filename, std::ios_base::app | std::ios_base::out);
//	file << client->username << endl;
//	file.close();
//
//	filename = ".\\database\\joined-group-by-username\\" + client->username + ".txt";
//	file.open(filename, std::ios_base::app | std::ios_base::out);
//	file << to_string(groupId) << endl;
//	file.close();
//
//	filename = ".\\database\\group-messages\\group" + to_string(groupId) + ".txt";
//	file.open(filename, std::ios_base::app | std::ios_base::out);
//	file.close();
//
//	filename = ".\\database\\unique-group-id.txt";
//	file.open(filename, std::ios_base::out);
//	file << autoIncrementGroupId;
//	file.close();
//
//	return string(CREATE_GROUP_SUCCESS) + " " + groupName;
//
//}
//
//
//string handleInviteRequest(string content, LPPER_HANDLE_DATA client) {
//	if (client->status == 0)
//		return string(ACCOUNT_NOT_LOGGED_IN);
//	string destUsername = content.substr(0, content.find(" "));
//	int groupId = atoi(content.substr(content.find(" ") + 1).c_str());
//
//	cout << destUsername << " " << groupId;
//
//	// check exception here ...
//	if (users.find(destUsername) == users.end())
//		return string(ACCOUNT_NOT_FOUND);
//
//	if (groups.find(groupId) == groups.end())
//		return "312";
//
//	string path = ".\\database\\group-members\\group" + to_string(groupId) + ".txt";
//	string line;
//	ifstream fin;
//	fin.open(path, std::ios_base::in);
//	if (!fin.is_open()) {
//		cerr << "Could not open file " << path << endl;
//		exit(1);
//	}
//
//	bool isSrcInGroup = false; // check if sender is in group
//	bool isDesInGroup = false; // check if receiver has already in group
//
//	while (getline(fin, line)) {
//		if (line.find(destUsername) != string::npos) {
//			isDesInGroup = true;
//		}
//			
//		if (line.find(client->username) != string::npos) {
//			isSrcInGroup = true;
//		}	
//	}
//	fin.close();
//
//	if (!isSrcInGroup)
//		return "341"; // sender not in group so cannot invite anyone
//	if (isDesInGroup)
//		return "313"; // receiver already in group
//
//	path = ".\\database\\invite-request\\" + destUsername + ".txt";
//	fin.open(path, std::ios_base::in);
//	while (getline(fin, line)) {
//		if (atoi(line.substr(0, line.find(" ")).c_str()) == groupId
//			&& line.substr(line.find(" ") + 1) == client->username)
//			return "314";
//	}
//	fin.close();
//
//	// sucess
//	ofstream fout;
//	fout.open(path, std::ios_base::out | std::ios_base::app);
//	fout << groupId << " " << client->username << endl;
//	fout.close();
//	return "310";
//
//}
//
//
//string handleGetInvitation(LPPER_HANDLE_DATA client) {
//	if (client->status == 0)
//		return string(ACCOUNT_NOT_LOGGED_IN);
//	
//	string path = ".\\database\\invite-request\\" + client->username + ".txt";
//	ifstream fin(path);
//	string line;
//
//	string respondBody = string(GET_INVITATION_SUCESS) + " ";
//	while (getline(fin, line)) {
//		int find = line.find(" ");
//		int groupId = atoi(line.substr(0, find).c_str());
//		string groupDisplayName = groups.at(groupId);
//		string fromUsername = line.substr(find + 1);
//
//		respondBody += to_string(groupId) + " " + groupDisplayName + " " + fromUsername + "/";
//	}
//
//	return respondBody;
//}
//
//
//string handleAcceptInvitation(string content, LPPER_HANDLE_DATA client) {
//	if (client->status == 0)
//		return string(ACCOUNT_NOT_LOGGED_IN);
//	int groupId = atoi(content.c_str()); // content is groupId in string
//	if (groups.find(groupId) == groups.end())
//		return "312";
//	string 	path = ".\\database\\group-members\\group" + content + ".txt";
//	ifstream fin(path);
//	string line;
//	vector<string> fileContent;
//	while (getline(fin, line)) {
//		if (line == client->username)
//			return "332";
//	}
//	fin.close();
//	
//	path = ".\\database\\invite-request\\" + client->username + ".txt";
//	fin.open(path, std::ios_base::in);
//	bool isInvited = false;
//	while (getline(fin, line)) {
//		fileContent.push_back(line);
//		if (content == line.substr(0, line.find(" ")))
//			isInvited = true;
//	}
//	fin.close();
//	if (!isInvited)
//		return "331";
//
//	// sucess:
//	path = ".\\database\\invite-request\\" + client->username + ".txt";
//	ofstream fout;
//	fout.open(path, std::ios_base::out | std::ios_base::trunc);
//	for (auto it = fileContent.begin(); it != fileContent.end(); it++) {
//		string line = *it;
//		if (line.find(content) != 0)
//			fout << line << endl;
//	}
//	fout.close();
//
//	path = ".\\database\\group-members\\group" + content + ".txt";
//	fout.open(path, std::ios_base::out | std::ios_base::app);
//	fout << client->username << endl;
//	fout.close();
//
//	path = ".\\database\\joined-group-by-username\\" + client->username + ".txt";
//	fout.open(path, std::ios_base::out | std::ios_base::app);
//	fout << content << endl;
//	fout.close();
//
//	return "330 " + content + " " + groups.at(groupId);
//
//}
//
//
//string handleDenyInvitation(string content, LPPER_HANDLE_DATA client) {
//	if (client->status == 0)
//		return string(ACCOUNT_NOT_LOGGED_IN);
//	int groupId = atoi(content.c_str()); // content is groupId in string
//	if (groups.find(groupId) == groups.end())
//		return "312";
//	string 	path = ".\\database\\group-members\\group" + content + ".txt";
//	ifstream fin(path);
//	string line;
//	vector<string> fileContent;
//	while (getline(fin, line)) {
//		if (line == client->username)
//			return "332";
//	}
//	fin.close();
//
//	path = ".\\database\\invite-request\\" + client->username + ".txt";
//	fin.open(path, std::ios_base::in);
//	bool isInvited = false;
//	while (getline(fin, line)) {
//		fileContent.push_back(line);
//		if (content == line.substr(0, line.find(" ")))
//			isInvited = true;
//	}
//	fin.close();
//	if (!isInvited)
//		return "331";
//
//	// sucess:
//	path = ".\\database\\invite-request\\" + client->username + ".txt";
//	ofstream fout;
//	fout.open(path, std::ios_base::out | std::ios_base::trunc);
//	for (auto it = fileContent.begin(); it != fileContent.end(); it++) {
//		string line = *it;
//		if (line.find(content) != 0)
//			fout << line << endl;
//	}
//	fout.close();
//
//	return "333 " + content + " " + groups.at(groupId);
//
//}
//
//
///*
//@author: LinhVD
//*/
//string handleListMemberGroupRequest(string content, LPPER_HANDLE_DATA client) {
//	for (int i = 0; i < content.length(); i++) {
//		if (content[i] < '0' || content[i] > '9') {
//			return "999";
//		}
//	}
//	int idGroupUser = atoi(content.c_str()); // content is groupId in string
//	string username = client->username;
//	if (client->status == 0) {
//		return "201";
//	}
//
//	if (groups.find(idGroupUser) == groups.end()) {
//		return "312";
//	}
//
//	string path = ".\\database\\group-members\\group" + content + ".txt";
//	ifstream fin;
//	fin.open(path, std::ios_base::in);
//	if (!fin.is_open()) {
//		cout << "Cannot open the file: " << path << endl;
//		return BAD_REQUEST;
//	}
//
//	vector<string>listMembersPerGroup;
//	string line;
//	while (getline(fin, line)) {
//		string memberUsername = line;
//		listMembersPerGroup.push_back(memberUsername);
//	}
//	fin.close();
//
//	string responseBody = "370";
//	bool isInGroup = false;
//	for (auto it = listMembersPerGroup.begin(); it != listMembersPerGroup.end(); it++) {
//		if (*it == client->username)
//			isInGroup = true;
//		responseBody += " " + *it;
//	}
//
//	if (!isInGroup)
//		return "341";
//	return responseBody;
//
//
//}
//
//
//
///*
//@author: LinhVD
//*/
//string handleLoadMessageRequest(string content, LPPER_HANDLE_DATA client) {
//	for (int i = 0; i < content.length(); i++) {
//		if (content[i] < '0' || content[i] > '9') {
//			return "999";
//		}
//	}
//	int idGroupUser = atoi(content.c_str());
//	string username = client->username;
//	if (client->status == 0) {
//		return "201";
//	}
//	if (groups.find(idGroupUser) == groups.end()) {
//		return "312";
//	}
//
//	string path = ".\\database\\group-members\\group" + content + ".txt";
//	ifstream fin;
//	fin.open(path, std::ios_base::in);
//	if (!fin.is_open()) {
//		cout << "Cannot open the file: " << path << endl;
//		return BAD_REQUEST;
//	}
//
//	vector<string>listMembersPerGroup;
//	string line;
//	while (getline(fin, line)) {
//		string memberUsername = line;
//		listMembersPerGroup.push_back(memberUsername);
//	}
//	fin.close();
//
//	bool isInGroup = false;
//
//	for (auto it = listMembersPerGroup.begin(); it != listMembersPerGroup.end(); it++) {
//		if (*it == client->username) {
//			isInGroup = true;
//			break;
//		}
//	}
//
//	if (!isInGroup)
//		return "341";
//
//	// if success
//	string responseBody = "380 " + to_string(idGroupUser) + " ";
//
//	string history;
//	path = ".\\database\\group-messages\\group" + content + ".txt";
//	fin.open(path, std::ios_base::in);
//	if (!fin.is_open()) {
//		cout << "Cannot open file " << path << endl;
//		return string(BAD_REQUEST);
//	}
//	while (getline(fin, line)) {
//		history += line + "\n";
//	}
//	fin.close();
//	responseBody.append(history);
//
//	return responseBody;
//
//
//}
//
//
//
///*
//@author: LinhVD
//*/
//string handleSendMessageRequest(string content, LPPER_HANDLE_DATA client) {
//	string sGroup = content.substr(0, content.find(" ")); // sGroup is groupId to chat in string
//
//	for (int i = 0; i < sGroup.length(); i++) {
//		if (sGroup[i] < '0' || sGroup[i] > '9') {
//			return "999";
//		}
//	}
//
//	int idGroupUser = atoi(sGroup.c_str());
//	string msg = content.substr(content.find(" ") + 1); // get message from user to chat in group
//	string username = client->username;
//	if (client->status == 0) {
//		return "201";
//	}
//	if (groups.find(idGroupUser) == groups.end()) {
//		return "312";
//	}
//
//	string path = ".\\database\\group-members\\group" + sGroup + ".txt";
//	ifstream fin;
//	fin.open(path, std::ios_base::in);
//	if (!fin.is_open()) {
//		cout << "Cannot open the file: " << path << endl;
//		return BAD_REQUEST;
//	}
//
//	vector<string>listMembersPerGroup;
//	string line;
//	while (getline(fin, line)) {
//		string memberUsername = line;
//		listMembersPerGroup.push_back(memberUsername);
//	}
//	fin.close();
//	string responseBody = "390 " + to_string(idGroupUser) + " " + username;
//	bool isInGroup = false;
//	for (auto it = listMembersPerGroup.begin(); it != listMembersPerGroup.end(); it++) {
//		if (*it == client->username) {
//			isInGroup = true;
//			break;
//		}
//	}
//
//	if (!isInGroup)
//		return "341";
//	
//
//	string oneMsg = username + ": " + msg;
//	path = ".\\database\\group-messages\\group" + sGroup + ".txt";
//	ofstream fout;
//	fout.open(path, std::ios_base::out | std::ios_base::app);
//	if (!fout.is_open()) {
//		cout << "Cannot open file " << path << endl;
//		return "999";
//	}
//	fout << oneMsg << endl;
//	fout.close();
//	responseBody += " " + oneMsg;
//	return responseBody;
//}
//
//
///*
//@author: DatNT
//*/
//string handleListGroupRequest(LPPER_HANDLE_DATA client) {
//	if (client->status == 0)
//		return string(ACCOUNT_NOT_LOGGED_IN);
//
//	string path = ".\\database\\joined-group-by-username\\" + client->username + ".txt";
//	ifstream file(path);
//	string line;
//	if (!file.is_open()) {
//		cerr << "Could not open file " << path << endl;
//		return string(LIST_GROUP_SUCCESS) + "/";
//	}
//
//	string responseBody = string(LIST_GROUP_SUCCESS) + " ";
//
//	while (getline(file, line)) {
//		int groupId = atoi(line.c_str());
//		if (groups.find(groupId) == groups.end())
//			continue;
//		responseBody += (line + " " + groups.at(groupId) + "/");
//
//	}
//
//	return responseBody;
//}
//
//
///*
//@author: DatNT
//*/
//string handleLeaveGroup(string content, LPPER_HANDLE_DATA client) {
//	if (client->status == 0)
//		return string(ACCOUNT_NOT_LOGGED_IN);
//
//	int groupId = atoi(content.c_str());
//	cout << groupId;
//
//	if (groups.find(groupId) == groups.end())
//		return "312";
//
//	vector<string> contentFile1, contentFile2;
//
//	string path = ".\\database\\group-members\\group" + to_string(groupId) + ".txt";
//	string line;
//	ifstream fin;
//	fin.open(path, std::ios_base::in);
//	if (!fin.is_open()) {
//		cerr << "Could not open file " << path << endl;
//		return "999"; // this should be another code
//	}
//
//	bool isSrcInGroup = false; // check if sender is in group
//	while (getline(fin, line)) {
//		contentFile1.push_back(line);
//		if (line.find(client->username) != string::npos) {
//			isSrcInGroup = true;
//		}
//	}
//	fin.close();
//
//	path = ".\\database\\joined-group-by-username\\" + client->username + ".txt";
//	fin.open(path, std::ios_base::in);
//	while (getline(fin, line))
//		contentFile2.push_back(line);
//	fin.close();
//
//
//	if (!isSrcInGroup)
//		return "341"; // sender not in group so cannot leave this group
//
//	// successfully leave group
//	path = ".\\database\\group-members\\group" + to_string(groupId) + ".txt";
//	ofstream fout;
//	fout.open(path, std::ios_base::out | std::ios_base::trunc);
//	for (auto it = contentFile1.begin(); it != contentFile1.end(); it++)
//		if (*it != client->username)
//			fout << *it << endl;
//	fout.close();
//
//	path = ".\\database\\joined-group-by-username\\" + client->username + ".txt";
//	fout.open(path, std::ios_base::out | std::ios_base::trunc);
//	for (auto it = contentFile2.begin(); it != contentFile2.end(); it++)
//		if (*it != content)
//			fout << *it << endl;
//	fout.close();
//	return "350";
//}
//
//
//
//
//
//
///// ###### FILTER REQUEST AND NAVIGATION TO SUITABLE FUNCTION ##################
//
//
//string outputResponseFrom(string request, LPPER_HANDLE_DATA client) {
//
//	/// PART 1
//
//	// LOGIN
//	if (request.find(LOGIN_REQ) != string::npos && request.find(LOGIN_REQ) == 0 
//		&& request.find(" ") == strlen(LOGIN_REQ)) {
//		request = request.substr(strlen(LOGIN_REQ) + 1);
//		return handleLoginRequest(request, client);
//	}
//
//	// SIGNUP
//	if (request.find(SIGNUP_REQ) != string::npos && request.find(SIGNUP_REQ) == 0
//		&& request.find(" ") == strlen(SIGNUP_REQ)) {
//		request = request.substr(strlen(SIGNUP_REQ) + 1);
//		return handleSignupRequest(request);
//	}
//
//	// LOGOUT
//	if (request.find(LOGOUT_REQ) != string::npos && request.find(LOGOUT_REQ) == 0) {
//		return handleLogoutRequest(client);
//	}
//
//
//	// CREATE NEW GROUP: CREATEGROUP ... (if not => bad request)
//	if (request.find(CREATE_GROUP_REQ) != string::npos && request.find(CREATE_GROUP_REQ) == 0
//		&& request.find(" ") == strlen(CREATE_GROUP_REQ)) {
//		request = request.substr(strlen(CREATE_GROUP_REQ) + 1);
//		return handleCreateGroupRequest(request, client);
//	}
//
//
//	/// PART 3
//
//	// INVITE TO NEW GROUP: INVITE username groupId (if not => bad request)
//	if (request.find(INVITE_REQ) != string::npos && request.find(INVITE_REQ) == 0
//		&& request.find(" ") == strlen(INVITE_REQ) ) {
//		request = request.substr(strlen(INVITE_REQ) + 1);
//		if (request.find(" ") != string::npos)
//			return handleInviteRequest(request, client);
//	}
//
//
//	// GET INVITATION
//	if (request.find(GET_INVITATION_REQ) != string::npos && request.find(GET_INVITATION_REQ) == 0) {
//		return handleGetInvitation(client);
//	}
//
//
//	// ACCEPT REQ
//	if (request.find(ACCEPT_REQ) != string::npos && request.find(ACCEPT_REQ) == 0
//		&& request.find(" ") == strlen(ACCEPT_REQ)) {
//		request = request.substr(strlen(ACCEPT_REQ) + 1);
//		return handleAcceptInvitation(request, client);
//	}
//
//	// DENY REQ
//	if (request.find(DENY_REQ) != string::npos && request.find(DENY_REQ) == 0
//		&& request.find(" ") == strlen(DENY_REQ)) {
//		request = request.substr(strlen(DENY_REQ) + 1);
//		return handleDenyInvitation(request, client);
//	}
//
//
//	// LIST JOINED GROUP of client
//	if (request.find(LIST_JOINED_GROUP_REQ) != string::npos && request.find(LIST_JOINED_GROUP_REQ) == 0) {
//		return handleListGroupRequest(client);
//	}
//
//	// LEAVE GROUP
//	if (request.find(LEAVE_GROUP_REQ) != string::npos && request.find(LEAVE_GROUP_REQ) == 0
//		&& request.find(" ") == strlen(LEAVE_GROUP_REQ)) {
//		request = request.substr(strlen(LEAVE_GROUP_REQ) + 1);
//		return handleLeaveGroup(request, client);
//	}
//
//
//	// LIST MEMBERS IN A GROUP
//	if (request.find(LIST_MEMBERS_GROUP) == 0) {//linh
//		request = request.substr(string(LIST_MEMBERS_GROUP).length() + 1);
//		return handleListMemberGroupRequest(request, client);
//	}
//
//
//	// RETURN ALL HISTORY CHAT IN GROUP
//	if (request.find(LOAD_MESSAGE) == 0) {//linh
//		request = request.substr(string(LOAD_MESSAGE).length() + 1);
//		return handleLoadMessageRequest(request, client);
//	}
//
//
//	// HANDLE WHEN AN USER SEND NEW MESSAGE
//	if (request.find(MESSAGE) == 0) {//linh
//		request = request.substr(string(MESSAGE).length() + 1);
//		return handleSendMessageRequest(request, client);
//	}
//
//	return string(BAD_REQUEST);
//}
//
//
//
//
//int main(int argc, CHAR* argv[])
//{
//	SOCKET acceptSock;
//
//	SOCKADDR_IN clientAddr;
//
//	LPSOCKET_IOCP iocp = new SOCKET_IOCP(); // server socket info and IOCP
//
//	SYSTEM_INFO systemInfo;
//
//	LPPER_HANDLE_DATA perHandleData;
//	LPPER_IO_OPERATION_DATA perIoData;
//
//	DWORD transferredBytes;
//	DWORD flags;
//
//	WSADATA wsaData;
//
//	// Step 0: Read file
//	readAccountDb();
//	readGroupDb();
//	readUniqueGroupId();
//
//
//	// Step 1: Init Winsock
//	if (WSAStartup((2, 2), &wsaData) != 0) {
//		printf("WSAStartup() failed with error %d\n", GetLastError());
//		return 1;
//	}
//
//	// Step 2: Create an I/O completion port
//	if ((iocp->completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0))
//		== NULL) {
//		printf("CreateIoCompletionPort() failed with error %d\n", GetLastError());
//		return 1;
//	}
//
//	// Step 3: Determine how many processors are on the system
//	GetSystemInfo(&systemInfo);
//
//	// Step 4: Create worker threads based on the number of processors available on the
//	// system. Create two worker threads for each processor	
//	for (int i = 0; i < (int)systemInfo.dwNumberOfProcessors * 2; i++) {
//		// Create a server worker thread and pass the completion port to the thread
//		if (_beginthreadex(0, 0, serverWorkerThread, (void*)iocp, 0, 0) == 0) {
//			printf("Create thread failed with error %d\n", GetLastError());
//			return 1;
//		}
//	}
//
//	// Step 5: Create a listening socket
//	if ((iocp->listenSock = WSASocketW(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED))
//		== INVALID_SOCKET) {
//		printf("WSASocket() failed with error %d\n", WSAGetLastError());
//		return 1;
//	}
//
//	// bind
//	iocp->serverAddr.sin_family = AF_INET;
//	iocp->serverAddr.sin_port = htons(PORT);
//	inet_pton(AF_INET, SERVER_ADDR, &iocp->serverAddr.sin_addr);
//	if (bind(iocp->listenSock, (PSOCKADDR)&iocp->serverAddr, sizeof(iocp->serverAddr)) == SOCKET_ERROR) {
//		printf("bind() failed with error %d\n", WSAGetLastError());
//		return 1;
//	}
//
//	// Prepare socket for listening
//	if (listen(iocp->listenSock, 10) == SOCKET_ERROR) {
//		printf("listen() failed with error %d\n", WSAGetLastError());
//		return 1;
//	}
//
//	cout << "Server start!\n";
//
//	while (1) {
//
//		// Step 6: Accept connections
//		if ((acceptSock = WSAAccept(iocp->listenSock, (SOCKADDR*)&clientAddr, NULL, NULL, 0)) == SOCKET_ERROR) {
//			printf("WSAAccept() failed with error %d\n", WSAGetLastError());
//			return 1;
//		}
//
//		iocp->clientSockList.push_back(acceptSock);
//
//		// Step 7: Create a socket information structure to associate with the socket
//		if ((perHandleData = (LPPER_HANDLE_DATA)GlobalAlloc(GPTR, sizeof(PER_HANDLE_DATA))) == NULL) {
//			printf("GlobalAlloc() failed with error %d\n", GetLastError());
//			return 1;
//		}
//		char clientIP[INET_ADDRSTRLEN];
//		cout << "Accept connection from IP Address: " + string(inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, sizeof(clientIP)))
//			+ " Port: " + to_string(ntohs(clientAddr.sin_port));
//
//		// Step 8: Associate the accepted socket with the original completion port
//		printf("\nSocket number %d got connected...\n", acceptSock);
//		perHandleData->socket = acceptSock; // tim 1 cho trong va dua vao mang clients: clients[i]->socket = acceptSock
//		perHandleData->status = 0;
//		memcpy(&(perHandleData->clientAddr), &clientAddr, sizeof(clientAddr));
//
//		if (CreateIoCompletionPort((HANDLE)acceptSock, iocp->completionPort, 
//			(DWORD)perHandleData, 0) == NULL) {
//			printf("CreateIoCompletionPort() failed with error %d\n", GetLastError());
//			return 1;
//		}
//
//		// Step 9: Create per I/O socket information structure to associate with the WSARecv call
//		if ((perIoData = (LPPER_IO_OPERATION_DATA)GlobalAlloc(GPTR, 
//			sizeof(PER_IO_OPERATION_DATA))) == NULL) {
//			printf("GlobalAlloc() failed with error %d\n", GetLastError());
//			return 1;
//		}
//
//		ZeroMemory(&(perIoData->overlapped), sizeof(OVERLAPPED));
//		perIoData->sentBytes = 0;
//		perIoData->recvBytes = 0;
//		perIoData->dataBuff.len = DATA_BUFSIZE;
//		perIoData->dataBuff.buf = perIoData->buffer;
//		perIoData->operation = RECEIVE;
//		flags = 0;
//
//		if (WSARecv(perHandleData->socket, &(perIoData->dataBuff), 1, &transferredBytes, &flags, &(perIoData->overlapped), NULL) == SOCKET_ERROR) {
//			if (WSAGetLastError() != ERROR_IO_PENDING) {
//				printf("WSARecv() failed with error %d\n", WSAGetLastError());
//				return 1;
//			}
//		}
//	}
//
//	// close socket:
//	for (list<SOCKET>::iterator it = iocp->clientSockList.begin();
//	it != iocp->clientSockList.end(); it++) {
//		closesocket(*it);
//	}
//	iocp->clientSockList.empty();
//	CloseHandle(iocp->completionPort);
//	
//	for (int i = 0; i < (int)systemInfo.dwNumberOfProcessors * 2; i++) {
//		PostQueuedCompletionStatus(iocp, 0, NULL, NULL);
//	}
//	closesocket(iocp->listenSock);
//	delete iocp;
//
//	WSACleanup();
//
//	_getch();
//	return 0;
//}
//
//
//
//unsigned __stdcall serverWorkerThread(LPVOID iocp)
//{
//	LPSOCKET_IOCP lpIocp = (LPSOCKET_IOCP)iocp;
//
//	DWORD transferredBytes;
//	LPPER_HANDLE_DATA perHandleData;
//	LPPER_IO_OPERATION_DATA perIoData;
//	DWORD flags;
//
//	string prevBuff;
//	queue<string> requestQueue;
//
//	while (TRUE) {
//		if (GetQueuedCompletionStatus(lpIocp->completionPort, &transferredBytes,
//			(LPDWORD)&perHandleData, (LPOVERLAPPED *)&perIoData, INFINITE) == 0) {
//			cout << "Client disconnect!" << endl;
//			handleLogoutRequest(perHandleData);
//			return 0;
//		}
//		// Check to see if an error has occurred on the socket and if so
//		// then close the socket and cleanup the SOCKET_INFORMATION structure
//		// associated with the socket
//		if (transferredBytes == 0) {
//			printf("Closing socket %d\n", perHandleData->socket);
//			if (closesocket(perHandleData->socket) == SOCKET_ERROR) {
//				printf("closesocket() failed with error %d\n", WSAGetLastError());
//				return 0;
//			}
//			GlobalFree(perHandleData);
//			GlobalFree(perIoData);
//
//			// free client socket
//			lpIocp->clientSockList.remove(perHandleData->socket);
//			closesocket(perHandleData->socket);
//			continue;
//		}
//		// Check to see if the operation field equals RECEIVE. If this is so, then
//		// this means a WSARecv call just completed so update the recvBytes field
//		// with the transferredBytes value from the completed WSARecv() call
//		if (perIoData->operation == RECEIVE) {
//			int found = 0;
//			string receivceBuffer = string(perIoData->buffer);
//			receivceBuffer = prevBuff + receivceBuffer;
//			prevBuff.clear();
//
//			// stream handle
//			while ((found = receivceBuffer.find("\r\n")) != string::npos) {
//				if (receivceBuffer.find("\r\n") > 0) {
//					requestQueue.push(receivceBuffer.substr(0, found));
//					receivceBuffer = receivceBuffer.substr(found + strlen(ENDING_DELIMITER));
//				}
//			}
//			if (receivceBuffer.length() > 0)
//				prevBuff = receivceBuffer;
//
//
//
//			// handle request
//			while (!requestQueue.empty()) {
//
//				cout << "Handle request from client: " << requestQueue.front() << endl;
//
//
//				// process
//				string response = outputResponseFrom(requestQueue.front(), perHandleData);
//				
//				response += ENDING_DELIMITER; // 390 1 datnt dlfjsldffsal fsdjf slfalsd f\r\n
//
//				// send response to one client
//				LPPER_IO_OPERATION_DATA perIoDataToSend = new PER_IO_OPERATION_DATA();
//				ZeroMemory(&(perIoDataToSend->overlapped), sizeof(OVERLAPPED));
//				memset(perIoDataToSend, 0, sizeof(PER_IO_OPERATION_DATA));
//				memcpy(perIoDataToSend->buffer, response.c_str(), response.length());
//				perIoDataToSend->dataBuff.buf = perIoDataToSend->buffer;
//				perIoDataToSend->dataBuff.len = response.length();
//				perIoDataToSend->operation = SEND;
//
//				DWORD bytesToSend = 0;
//
//				if (WSASend(perHandleData->socket, &(perIoDataToSend->dataBuff), 1,
//					&bytesToSend, 0, &perIoDataToSend->overlapped, NULL) == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
//					if (perHandleData != NULL) {
//						closesocket(perHandleData->socket);
//						lpIocp->clientSockList.remove(perHandleData->socket);
//					}
//
//
//				string flag = response.substr(0, 3);
//				
//
//				if (flag == "390") {
//					//// group chat broadcast 
//					vector<string>p = simple_tokenizer(response);
//					int gr = atoi(p[1].c_str());
//
//					string newResponse = "381 " + p[1] + " ";
//					response = response.substr(response.find(" ") + 1);
//					response = response.substr(response.find(" ") + 1);
//					response = response.substr(response.find(" ") + 1);
//
//					newResponse += response; // "381 groupId content...\r\n"
//
//
//					string path = ".\\database\\group-members\\group" + to_string(gr) + ".txt";
//					ifstream fin;
//					fin.open(path, std::ios_base::in);
//					if (!fin.is_open()) {
//						cout << "Cannot open the file: " << path << endl;
//					}
//
//					vector<string>listMembersPerGroup;
//					string line;
//					while (getline(fin, line)) {
//						string memberUsername = line;
//						listMembersPerGroup.push_back(memberUsername);
//					}
//					fin.close();
//
//
//					for (int it1 = 0; it1 < listMembersPerGroup.size(); it1++) {
//						auto info = users.at(listMembersPerGroup[it1]);
//						if (get<2>(info) == 1 && perHandleData->username != p[2]) {
//							LPPER_IO_OPERATION_DATA perIoDataToSend = new PER_IO_OPERATION_DATA();
//							ZeroMemory(&(perIoDataToSend->overlapped), sizeof(OVERLAPPED));
//							memset(perIoDataToSend, 0, sizeof(PER_IO_OPERATION_DATA));
//							memcpy(perIoDataToSend->buffer, newResponse.c_str(), newResponse.length());
//							perIoDataToSend->dataBuff.buf = perIoDataToSend->buffer;
//							perIoDataToSend->dataBuff.len = newResponse.length();
//							perIoDataToSend->operation = SEND;
//
//							DWORD bytesToSend = 0;
//							if (WSASend(get<1>(info), &(perIoDataToSend->dataBuff), 1,
//								&bytesToSend, 0, &perIoDataToSend->overlapped, NULL) == SOCKET_ERROR
//								&& WSAGetLastError() != WSA_IO_PENDING) {
//								if (perHandleData != NULL) {
//									if (get<1>(info) != NULL) {
//										closesocket(get<1>(info));
//										lpIocp->clientSockList.remove(perHandleData->socket);
//									}
//								}
//							}
//						}
//					}
//				}
//
//
//
//				requestQueue.pop();
//			}
//
//
//			// server continue being at state RECEIVE
//			ZeroMemory(&(perIoData->overlapped), sizeof(OVERLAPPED));
//			memset(perIoData, 0, sizeof(PER_IO_OPERATION_DATA));
//			perIoData->dataBuff.buf = perIoData->buffer;
//			perIoData->dataBuff.len = sizeof(perIoData->buffer);
//			perIoData->operation = RECEIVE;
//			DWORD bytesToReceive = 0, flag = 0;
//			WSARecv(perHandleData->socket, &(perIoData->dataBuff), 1, &bytesToReceive,
//				&flag, &(perIoData->overlapped), NULL);
//
//			continue;
//		}
//		else {
//			// WSASend() success
//			if (perIoData != NULL) {
//				delete perIoData;
//				perIoData = NULL;
//			}
//			continue;
//		}
//	} // end while (1)
//	return 0;
//}