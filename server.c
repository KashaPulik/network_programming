#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>

#define BUFLEN 81

int main()
{
    int sockMain, msgLength;
    socklen_t length;
    struct sockaddr_in servAddr, clientAddr;
    char buf[BUFLEN];

    if ((sockMain = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Сервер не может открыть socket для UDP.");
        return 1;
    }

    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = INADDR_ANY;
    servAddr.sin_port = 0;

    if (bind(sockMain, (struct sockaddr*)&servAddr, sizeof(servAddr))) {
        perror("Связывание сервера неудачно.");
        return 1;
    }

    length = sizeof(servAddr);

    if (getsockname(sockMain, (struct sockaddr*)&servAddr, &length)) {
        perror("Вызов getsockname неудачен.");
        return 1;
    }

    printf("SERVER: номер порта - %d\n", servAddr.sin_port);

    for (;;) {
        length = sizeof(clientAddr);

        if ((msgLength = recvfrom(sockMain, buf, BUFLEN, 0, (struct sockaddr*)&clientAddr, &length)) < 0) {
            perror("Плохой socket клиента.");
            return 1;
        }
        if (sendto(sockMain, buf, BUFLEN, 0, (struct sockaddr*)&clientAddr, length) < 0) {
            perror("Проблемы с sendto.\n");
            return 1;
        }
        printf("SERVER: IP адрес клиента: %s\n", inet_ntoa(clientAddr.sin_addr));
        printf("SERVER: PORT клиента: %d\n", ntohs(clientAddr.sin_port));
        printf("SERVER: Длина сообщения - %d\n", msgLength);
        printf("SERVER: Сообщение : %s\n\n", buf);
    }
}