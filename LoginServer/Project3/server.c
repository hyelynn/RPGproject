#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS


#include <WinSock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>
#include <process.h>
#include "point.h"
#include "server.h"
#include "member.h"
#include "query.h"
#include <mysql.h>
#define BUFSIZE  128
#pragma comment(lib, "ws2_32")



void err_exit(const char* msg) {
	LPVOID lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, 0, WSAGetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&lpMsgBuf, 0, 0);
	MessageBox(NULL, (LPCSTR)lpMsgBuf, msg, MB_OK);
	LocalFree(lpMsgBuf);
	exit(1);
}

int recvn(SOCKET sock, char* buffer, int length, int flags) {
	int curLen = 0;
	while (curLen < length) {
		int recvLen = recv(sock, buffer, length, flags);
		if (recvLen < 1)
			return SOCKET_ERROR;
		buffer += recvLen;
		curLen += recvLen;
	}
	return curLen;
}


void ErrorHandling(char* msg) {
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}




int main(int argc, char* argv[]) {
	
	
	WSADATA wsaData;
	SOCKET serverSock, clientSock;
	SOCKADDR_IN serverAddr, clientAddr;
	int clientAddrSize;
	HANDLE hThread;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) //윈도우 소켓을 사용하겠다는 사실을 운영체제에 전달
		ErrorHandling("WSAStartup() error!");

	hMutex = CreateMutex(NULL, FALSE, NULL);//하나의 뮤텍스를 생성한다.
	serverSock = socket(PF_INET, SOCK_STREAM, 0); //하나의 소켓을 생성한다.

	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(PORT);

	if (bind(serverSock, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) //생성한 소켓을 배치한다.
		ErrorHandling("bind() error");
	puts("bind성공 !\n");
	if (listen(serverSock, 5) == SOCKET_ERROR)//소켓을 준비상태에 둔다.
		ErrorHandling("listen() error");

	while (1) {
		clientAddrSize = sizeof(clientAddr);
		clientSock = accept(serverSock, (SOCKADDR*)&clientAddr, &clientAddrSize);//서버에게 전달된 클라이언트 소켓을 clientSock에 전달
		WaitForSingleObject(hMutex, INFINITE);//뮤텍스 실행
		clientSocks[clientCount++] = clientSock;//클라이언트 소켓배열에 방금 가져온 소켓 주소를 전달
		ReleaseMutex(hMutex);//뮤텍스 중지
		hThread = (HANDLE)_beginthreadex(NULL, 0, HandleClient, (void*)&clientSock, 0, NULL);//HandleClient 쓰레드 실행, clientSock을 매개변수로 전달
		printf("Connected Client IP : %s\n", inet_ntoa(clientAddr.sin_addr));
	}
	closesocket(serverSock);//생성한 소켓을 끈다.
	WSACleanup();//윈도우 소켓을 종료하겠다는 사실을 운영체제에 전달
	return 0;
}

unsigned WINAPI HandleClient(void* arg) {
	SOCKET clientSock = *((SOCKET*)arg); //매개변수로받은 클라이언트 소켓을 전달
	int strLen = 0, i;
	int type;
	char msg[BUF_SIZE];
	int k = 1;
	char query[BUFSIZ] = "";
	while ((strLen = recvn(clientSock, (char*)&type, sizeof(int), 0)) != 0) { //클라이언트로부터 메시지를 받을때까지 기다린다.
	
		RecvPoint recvPoint;
		SendPoint sendPoint;
		SendMemberInfo smInfo;
		MemberInfo mInfo;
		Query q ;
		int count;
		int recvLen;
		printf("%d", type);
		switch (type) {
		case REGISTER :
			q = getConnection();
			recvLen = recvn(clientSock, (char*)&mInfo, sizeof(MemberInfo), 0);
			if (recvLen < 0) {
				perror("recvn");
				DestroyConnection(q.conn);
				return -1;
			}
			RESISTERUSER(query, mInfo);
			if (mysql_query(q.conn, query) != 0)   //show변수를 query에 넣어준것이죠. 즉, 실행한거죠   
			{
				printf("query fail; err=%s\n", mysql_error(q.conn));
				//만약 에러가 생겼을 때 다음을 출력하는거랍니다.  
				DestroyConnection(q.conn);
				return -1;
			}
			DestroyConnection(q.conn);
			smInfo = MemInfoToSendMemInfo(REGISTER, mInfo);
			send(*(SOCKET*)arg, (char*)&smInfo, sizeof(smInfo),0);
			
			break;
		case LOGIN:
			q = getConnection();
			recvLen = recvn(clientSock, (char*)&mInfo, sizeof(MemberInfo), 0);
			if (recvLen < 0) {
				perror("recvn");
				DestroyConnection(q.conn);
			}
			ISEXIST(query, mInfo);
			if (mysql_query(q.conn, query) != 0)   //show변수를 query에 넣어준것이죠. 즉, 실행한거죠   
			{
				printf("query fail; err=%s\n", mysql_error(q.conn));
				//만약 에러가 생겼을 때 다음을 출력하는거랍니다.  
				DestroyConnection(q.conn);
				return -1;
			}
			if ((count = CountDupResult(q)) != 1) {
				mInfo.id = MAXINT;
				mInfo.password = MAXINT;
			}
			DestroyConnection(q.conn);
			smInfo = MemInfoToSendMemInfo(LOGIN, mInfo);
			send(*(SOCKET*)arg, (char*)&smInfo, sizeof(smInfo), 0);
			break;
		case SPAWN:
			break;
		case MOVE: 
			recvLen = recvn(clientSock, (char*)&recvPoint, sizeof(recvPoint), 0);
			sendPoint = recvToSend(MOVE, recvPoint);
			int j;
			WaitForSingleObject(hMutex, INFINITE);//뮤텍스 실행
			for (j = 0; j < clientCount; j++)//클라이언트 개수만큼
				send(clientSocks[j], (char*)&sendPoint, sizeof(sendPoint), 0);//클라이언트들에게 메시지를 전달한다.
			ReleaseMutex(hMutex);//뮤텍스 중지
			break;

		case DUPCHECK :
			q = getConnection();
			recvLen = recvn(clientSock, (char*)&mInfo.id, sizeof(int), 0);

			if (recvLen < 0) {
				perror("recvn");
				DestroyConnection(q.conn);
				return -1;
			}
			ISDUP(query, mInfo.id);
			if (mysql_query(q.conn, query) != 0)   //show변수를 query에 넣어준것이죠. 즉, 실행한거죠   
			{
				printf("query fail; err=%s\n", mysql_error(q.conn));
				//만약 에러가 생겼을 때 다음을 출력하는거랍니다.  
				DestroyConnection(q.conn);
				return -1;
			}
			count = CountDupResult(q);
			DestroyConnection(q.conn);
			send(*(SOCKET*)arg,(char*)&count, sizeof(count), 0);
			break;

		case ITEM:
			
			q = getConnection();
			int newBuf[][BUFSIZ] = {0,};
			GETITEM(query);
			if (mysql_query(q.conn, query) != 0)   //show변수를 query에 넣어준것이죠. 즉, 실행한거죠   
			{
				printf("query fail; err=%s\n", mysql_error(q.conn));
				//만약 에러가 생겼을 때 다음을 출력하는거랍니다.  
				DestroyConnection(q.conn);
				return -1;
			}
			q.res = mysql_store_result(q.conn);

			int count = mysql_num_rows(q.res);
			int k = 0;
			send(*(SOCKET*)arg, (char*)&count, sizeof(count), 0);
			while ((q.row = mysql_fetch_row(q.res)) != NULL) {
				printf("%s %s %s %s %s %s\n", q.row[0],q.row[1], q.row[2], q.row[3], q.row[4], q.row[5] );
				newBuf[1] = { 2,2,2,2,2,2 };
				newBuf[k] = { atoi(q.row[0]),atoi(q.row[1]),atoi(q.row[2]),atoi(q.row[3]),atoi(q.row[4]),atoi(q.row[5]) };
				send(*(SOCKET*)arg, (char*)&newBuf, sizeof(newBuf), 0);
				k++;
			}
			
			
			
			mysql_free_result(q.res);

			break;
		
		}
	}




	//이 줄을 실행한다는 것은 해당 클라이언트가 나갔다는 사실임 따라서 해당 클라이언트를 배열에서 제거해줘야함
	WaitForSingleObject(hMutex, INFINITE);//뮤텍스 실행
	for (i = 0; i < clientCount; i++) {//배열의 갯수만큼
		if (clientSock == clientSocks[i]) {//만약 현재 clientSock값이 배열의 값과 같다면
			while (i++ < clientCount - 1)//클라이언트 개수 만큼
				clientSocks[i] = clientSocks[i + 1];//앞으로 땡긴다.
			break;
		}
	}
	clientCount--;//클라이언트 개수 하나 감소
	ReleaseMutex(hMutex);//뮤텍스 중지
	closesocket(clientSock);//소켓을 종료한다.
	return 0;
}
//중계





