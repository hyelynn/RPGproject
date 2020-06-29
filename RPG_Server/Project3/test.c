#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <WinSock2.h>
#pragma comment(lib, "ws2_32")

//#define PORT 4567
#define PORT 4000
typedef struct SendPoint {
   int type, x, y, z;
}SendPoint;

typedef struct RecvPoint{
   int x, y, z;
}RecvPoint;

SendPoint createPoint3(int type,int x, int y, int z) {
   SendPoint v = { type,x,y,z };
   return v;
}

SendPoint createPoint3_() {
   int x, y, z;
   int type;
   printf("type: "); scanf("%d", &type);
   printf("x: "); scanf("%d", &x);
   printf("y: "); scanf("%d", &y);
   printf("z: "); scanf("%d", &z);
   return createPoint3(type,x, y, z);
}

void err_exit(const char* msg) {
   LPVOID lpMsgBuf;
   FormatMessage(
      FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
      0, WSAGetLastError(),
      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
      (LPSTR)&lpMsgBuf, 0, 0);
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


int main(int argc, char* argv[]) {
   printf("c server\n");
   WSADATA wsa;
   WSAStartup(MAKEWORD(2, 2
   ), &wsa);         //WS2_32.DLL�� ����Ҽ� �ֵ��� �ʱ�ȭ �ϴµ� ����ϴ� �Լ�. �Ʒ��� WSACleanup()�� ���� �ݾ��ִ°͵� ����.

   SOCKET gate = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);   //socket()�� ���� ������ ������ ������. TCP��

   if (gate == INVALID_SOCKET) {   //���� ���� ���� ����������
      err_exit("gate");
   }
   SOCKADDR_IN gateAddress = { 0, };
   gateAddress.sin_family = AF_INET;      //�ּ� ü��: AF_INET �׻� �̰����� �����ؾ���. �ʼ��� // IPV4 ���ͳ� ��������
   gateAddress.sin_addr.s_addr = htonl(INADDR_ANY);   //ȣ��ƮIP�ּ� -> �� �������� INADDR_ �� �����ϴ� ��, ���� ��� 'INADDR_ANY' �� ���� ���� ����Ǿ�� �Ѵ�. // 32��Ʈ IPV4�ּ�
   gateAddress.sin_port = htons(PORT);         //   short���� ������ host byte order���� network byte order�� �ٲ���, �� �̺����� ����Ǵ� ���� network byte order�̾����.

   if (bind(gate, (const struct sockaddr*)&gateAddress, sizeof(gateAddress)) == SOCKET_ERROR) { //bind()�Լ��� ����. ��, Ŭ���̾�Ʈ�� ������ ��ġ�� �˱����� IP�� port�� ����۾� �̶󺸸��.
      err_exit("bind");
   }

   if (listen(gate, SOMAXCONN) == SOCKET_ERROR) {   //Ŭ���̾�Ʈ�� connect()�Լ��� ���� ��û�� 'Ȯ��'�ϵ��� ��û�Ѵ�. Ȯ�εǾ��� ��û�� ���� ó���� accept()�Լ����� �ѱ��. 5�پƷ�������
      err_exit("listen");
   }

   SOCKADDR_IN clientAddress = { 0, };
   int cliAddrSize = sizeof(clientAddress);
   SOCKET client = accept(gate, (struct sockaddr*)&clientAddress, &cliAddrSize);   //���ӵ� Ŭ���̾�Ʈ�� ��ȭ�ϱ� ���� ���� ����


   


   if (client == INVALID_SOCKET) err_exit("client");
   printf("accepted\n");

   printf("send\n");
   SendPoint sendPoint3 = createPoint3_();
   int sendLen = send(client, (char*)&sendPoint3, sizeof(sendPoint3), 0);
   if (sendLen < 1)err_exit("send");

   int a = -1;
   int recvLen1 = recvn(client, (char*)&a, sizeof(int), 0);
   printf("%d", a);
   RecvPoint recvPoint3;
   int recvLen = recvn(client, (char*)&recvPoint3, sizeof(recvPoint3), 0);
   if (recvLen < 1)err_exit("recv");
   printf("recv \nx: %d \ny: %d \nz: %d\n", recvPoint3.x, recvPoint3.y, recvPoint3.z);

   closesocket(client);      //�� �� ���� �ݱ�
   closesocket(gate);
   WSACleanup();
   system("pause");
}