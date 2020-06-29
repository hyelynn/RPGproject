#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <WinSock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>
#include <process.h>
#include"point.h"
#include "server.h"
#pragma comment(lib, "ws2_32")
void err_exit(const char* msg) {
    LPVOID lpMsgBuf;
    FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, 0, WSAGetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&lpMsgBuf, 0, 0);
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
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) //������ ������ ����ϰڴٴ� ����� �ü���� ����
        ErrorHandling("WSAStartup() error!");

    hMutex = CreateMutex(NULL, FALSE, NULL);//�ϳ��� ���ؽ��� �����Ѵ�.
    serverSock = socket(PF_INET, SOCK_STREAM, 0); //�ϳ��� ������ �����Ѵ�.

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(PORT);

    if (bind(serverSock, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) //������ ������ ��ġ�Ѵ�.
        ErrorHandling("bind() error");
    if (listen(serverSock, 5) == SOCKET_ERROR)//������ �غ���¿� �д�.
        ErrorHandling("listen() error");

    while (1) {
        clientAddrSize = sizeof(clientAddr);
        clientSock = accept(serverSock, (SOCKADDR*)&clientAddr, &clientAddrSize);//�������� ���޵� Ŭ���̾�Ʈ ������ clientSock�� ����
        WaitForSingleObject(hMutex, INFINITE);//���ؽ� ����
        clientSocks[clientCount++] = clientSock;//Ŭ���̾�Ʈ ���Ϲ迭�� ��� ������ ���� �ּҸ� ����
        ReleaseMutex(hMutex);//���ؽ� ����
        hThread = (HANDLE)_beginthreadex(NULL, 0, HandleClient, (void*)&clientSock, 0, NULL);//HandleClient ������ ����, clientSock�� �Ű������� ����
        printf("Connected Client IP : %s\n", inet_ntoa(clientAddr.sin_addr));
    }
    closesocket(serverSock);//������ ������ ����.
    WSACleanup();//������ ������ �����ϰڴٴ� ����� �ü���� ����
    return 0;
}

unsigned WINAPI HandleClient(void* arg) {
    SOCKET clientSock = *((SOCKET*)arg); //�Ű������ι��� Ŭ���̾�Ʈ ������ ����
    int strLen = 0, i,type;
    char msg[BUF_SIZE];

    while ((strLen = recvn(clientSock, (char*)&type, sizeof(int), 0)) != 0) { //Ŭ���̾�Ʈ�κ��� �޽����� ���������� ��ٸ���.

        RecvPoint recvPoint;
        SendPoint sendPoint;
        int recvLen;
        switch (type) {
        case REGISTER:
            break;
        case LOGIN:
            break;
        case SPAWN:
            break;
        case MOVE:
            recvLen = recvn(clientSock, (char*)&recvPoint, sizeof(recvPoint), 0);
            sendPoint = recvToSend(MOVE, recvPoint);
            int j;
            WaitForSingleObject(hMutex, INFINITE);//���ؽ� ����
            for (j = 0; j < clientCount; j++)//Ŭ���̾�Ʈ ������ŭ
                send(clientSocks[j], (char*)&sendPoint, sizeof(sendPoint), 0);//Ŭ���̾�Ʈ�鿡�� �޽����� �����Ѵ�.
            ReleaseMutex(hMutex);//���ؽ� ����
            break;
        }
    }
        



    //�� ���� �����Ѵٴ� ���� �ش� Ŭ���̾�Ʈ�� �����ٴ� ����� ���� �ش� Ŭ���̾�Ʈ�� �迭���� �����������
    WaitForSingleObject(hMutex, INFINITE);//���ؽ� ����
    for (i = 0; i < clientCount; i++) {//�迭�� ������ŭ
        if (clientSock == clientSocks[i]) {//���� ���� clientSock���� �迭�� ���� ���ٸ�
            while (i++ < clientCount - 1)//Ŭ���̾�Ʈ ���� ��ŭ
                clientSocks[i] = clientSocks[i + 1];//������ �����.
            break;
        }
    }
    clientCount--;//Ŭ���̾�Ʈ ���� �ϳ� ����
    ReleaseMutex(hMutex);//���ؽ� ����
    closesocket(clientSock);//������ �����Ѵ�.
    return 0;
}
//�߰�





