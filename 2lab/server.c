#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFLEN 81
void BuffWork(int sockClient)
{
    char buf[BUFLEN];
    int msgLength;
    bzero(buf, BUFLEN);
    if ((msgLength = recv(sockClient, buf, BUFLEN, 0)) < 0) {
        perror("Плохое получение дочерним процессом.");
        exit(1);
    }
    printf("SERVER: Socket для клиента - %d\n", sockClient);
    printf("SERVER: Длина сообщения - %d\n", msgLength);
    printf("SERVER: Сообщение: %s\n\n", buf);
}

int main()
{
    int sockMain, sockClient;
    socklen_t length;
    struct sockaddr_in servAddr;
    if ((sockMain = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Сервер не может открыть главный socket.");
        exit(1);
    }
    bzero((char*)&servAddr, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = 0;
    if (bind(sockMain, (struct sockaddr*)&servAddr, sizeof(servAddr))) {
        perror("Связывание сервера неудачно.");
        exit(1);
    }
    length = sizeof(servAddr);
    if (getsockname(sockMain, (struct sockaddr*)&servAddr, &length)) {
        perror("Вызов getsockname неудачен.");
        exit(1);
    }
    printf("СЕРВЕР: номер порта - % d\n", ntohs(servAddr.sin_port));
    listen(sockMain, 5);
    for (;;) {
        if ((sockClient = accept(sockMain, 0, 0)) < 0) {
            perror("Неверный socket для клиента.");
            exit(1);
        }
        BuffWork(sockClient);
        close(sockClient);
    }
}