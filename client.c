#include <arpa/inet.h>
#include <errno.h>
#include <features.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFLEN 81

int main(int argc, char* argv[])
{
    int sock, number = atoi(argv[3]);
    socklen_t length;
    struct sockaddr_in servAddr, clientAddr;
    struct hostent *hp, *gethostbyname();
    char buf[BUFLEN];
    if (argc < 4) {
        printf("ВВЕСТИ udpclient имя_хоста порт сообщение\n");
        return 1;
    }
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("He могу получить socket\n");
        return 1;
    }
    bzero((char*)&servAddr, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    hp = gethostbyname(argv[1]);
    bcopy(hp->h_addr_list[0], &servAddr.sin_addr, hp->h_length);
    servAddr.sin_port = htons(atoi(argv[2]));
    bzero((char*)&clientAddr, sizeof(clientAddr));
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    clientAddr.sin_port = 0;
    if (bind(sock, (struct sockaddr*)&clientAddr, sizeof(clientAddr))) {
        perror("Клиент не получил порт.");
        return 1;
    }
    printf("CLIENT: Готов к пересылке.\n\n");
    for (int i = 0; i < number * number; i++) {
        length = sizeof(servAddr);

        printf("Отправка сообщения: %s\n", argv[3]);
        if (sendto(sock, argv[3], strlen(argv[3]), 0, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0) {
            perror("Проблемы с sendto.\n");
            return 1;
        }
        if (recvfrom(sock, buf, BUFLEN, 0, (struct sockaddr*)&servAddr, &length) < 0) {
            perror("Плохой socket сервера.\n");
            return 1;
        }
        printf("Сообщение от сервера: %s\n\n", buf);
        sleep(number);
    }
    printf("CLIENT: Пересылка завершена. Счастливо оставаться.\n");
    close(sock);
}