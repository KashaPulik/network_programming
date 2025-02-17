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
    if (argc < 4) {
        printf("ВВЕСТИ client имя_хоста порт число\n");
        return 1;
    }

    int sock, number = atoi(argv[3]);
    socklen_t length;
    struct sockaddr_in servAddr, clientAddr;
    char buf[BUFLEN];

    struct hostent* hp = NULL;

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("He могу получить socket\n");
        return 1;
    }

    if ((hp = gethostbyname(argv[1])) == NULL) {
        perror("Хост не доступен или не существует\n");
        return 1;
    }

    memcpy(&servAddr.sin_addr, hp->h_addr_list[0], hp->h_length);
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(atoi(argv[2]));
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_port = 0;

    if (bind(sock, (struct sockaddr*)&clientAddr, sizeof(clientAddr))) {
        perror("Клиент не получил порт.");
        return 1;
    }

    printf("CLIENT: Готов к пересылке.\n\n");
    for (int i = 0; i < number * number; i++) {
        length = sizeof(clientAddr);

        printf("Отправка сообщения: %s\n", argv[3]);
        
        if (sendto(sock, argv[3], strlen(argv[3]), 0, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0) {
            perror("Проблемы с sendto.\n");
            return 1;
        }

        if ((recvfrom(sock, buf, BUFLEN, 0, (struct sockaddr*)&clientAddr, &length)) < 0) {
            perror("Плохой socket сервера.");
            return 1;
        }

        printf("Ответ от сервера: %s\n\n", buf);

        sleep(number);
    }
    printf("CLIENT: Пересылка завершена. Счастливо оставаться.\n");
    close(sock);
}