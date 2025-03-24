#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUFLEN 1024
#define MAX_CLIENTS 2

typedef struct sockaddr_in sockaddr_in;
typedef struct sockaddr sockaddr;

sockaddr_in get_some_address()
{
    sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = 0;
    return address;
}

int my_bind(int fd, sockaddr_in* address)
{
    return bind(fd, (sockaddr*)address, sizeof(*address));
}

int my_getsockname(int fd, sockaddr_in* address)
{
    socklen_t address_len = sizeof(*address);
    return getsockname(fd, (sockaddr*)address, &address_len);
}

int handler(int client_socket)
{
    char buffer[BUFLEN];
    int messege_len;
    sockaddr_in client_address;
    socklen_t client_address_len = sizeof(client_address);

    memset(buffer, '\0', BUFLEN);

    messege_len = recv(client_socket, buffer, BUFLEN, 0);

    if (getpeername(client_socket, (sockaddr*)&client_address, &client_address_len) == 0) {
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(client_address.sin_addr), client_ip, INET_ADDRSTRLEN);
        printf("[%s:%d]: ", client_ip, ntohs(client_address.sin_port));
        if (messege_len == 0)
            printf("Client disconnected\n\n");
        else
            printf("%s\n\n", buffer);
    } else {
        perror("Error: can't get client address");
    }

    return messege_len;
}

int main()
{
    int main_socket;

    fd_set rfds, afds;
    int fd, nfds, client_socket;

    sockaddr_in client_address;
    socklen_t client_address_len = sizeof(client_address);

    if ((main_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Error: can't open main socket");
        return 1;
    }

    sockaddr_in server_address = get_some_address();

    if (my_bind(main_socket, &server_address) < 0) {
        perror("Error: can't bind socket");
        return 1;
    }

    if (my_getsockname(main_socket, &server_address) < 0) {
        perror("Error: getsockname fall");
        return 1;
    }

    printf("Server works on port %d\n\n", ntohs(server_address.sin_port));

    if (listen(main_socket, MAX_CLIENTS) < 0) {
        perror("Error: listen fail");
        return 1;
    }

    nfds = getdtablesize();
    FD_ZERO(&afds);
    FD_SET(main_socket, &afds);

    for (;;) {
        memcpy(&rfds, &afds, sizeof(rfds));

        if (select(nfds, &rfds, (fd_set*)0, (fd_set*)0, (struct timeval*)0) < 0) {
            perror("Error: select fall");
            return 1;
        }

        if (FD_ISSET(main_socket, &rfds)) {
            if ((client_socket = accept(main_socket, (sockaddr*)&client_address, &client_address_len)) < 0) {
                perror("Error: client acception fail");
                return 1;
            }
            FD_SET(client_socket, &afds);
        }

        for (fd = 0; fd < nfds; fd++) {
            printf("%d\n", fd);
            if (fd != main_socket && FD_ISSET(fd, &rfds)) {
                if (handler(fd) == 0) {
                    close(fd);
                    FD_CLR(fd, &afds);
                }
            }
        }
    }
}
