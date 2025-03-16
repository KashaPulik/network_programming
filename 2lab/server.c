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

int communication(int client_socket, sockaddr_in client_address)
{
    char buffer[BUFLEN];
    int messege_len;

    memset(buffer, '\0', BUFLEN);

    messege_len = recv(client_socket, buffer, BUFLEN, 0);

    if (messege_len < 0) {
        perror("Error: can't recieve messege from client");
        return -1;
    } else if (messege_len == 0) {
        printf("Client on port %d disconnected\n\n", ntohs(client_address.sin_port));
        return 1;
    } else {
        printf("Recieved messege \"%s\" from client on port %d\n\n", buffer, ntohs(client_address.sin_port));
        return 0;
    }
}

int client_process(int main_socket)
{
    int client_socket;
    sockaddr_in client_address;
    socklen_t client_address_len = sizeof(client_address);

    if ((client_socket = accept(main_socket, (sockaddr*)&client_address, &client_address_len)) < 0) {
        perror("Error: client acception fail");
        return 1;
    }

    int child = fork();

    if (child == -1) {
        perror("Error: can't create new process");
        close(main_socket);
        close(client_socket);
        return 1;
    } else if (child == 0) {
        close(main_socket);
        for (;;) {
            switch (communication(client_socket, client_address)) {
            case -1:
                close(client_socket);
                exit(1);
            case 1:
                close(client_socket);
                exit(0);
            default:
                break;
            }
        }
    }
    close(client_socket);
    return 0;
}

void reaper(int sig)
{
    int status;
    while (waitpid(-1, &status, WNOHANG) > 0)
        ;
}

int main()
{
    int main_socket;

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

    signal(SIGCHLD, reaper);

    for (;;) {
        if (client_process(main_socket) == 1)
            return 1;
    }
}
