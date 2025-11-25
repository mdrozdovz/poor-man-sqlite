//
// Created by mdrozdov on 30.10.25.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>

#include <sys/socket.h>
#include <netinet/in.h>

#include "server.h"

#define BUFFER_SIZE 1024

int start_server(unsigned short port, int backlog) {
    struct sockaddr_in server_info = {0};
    server_info.sin_addr.s_addr = 0;
    server_info.sin_family = AF_INET;
    server_info.sin_port = htons(port);

    const int sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd == -1) {
        perror("socket");
        return -1;
    }

    if (bind(sfd, (struct sockaddr *) &server_info, sizeof(struct sockaddr_in)) == -1) {
        perror("bind");
        return -1;
    }

    if (listen(sfd, backlog) == -1) {
        perror("listen");
        return -1;
    }

    printf("Listening for incoming connections on port: %d\n", port);

    return sfd;
}

int accept_connections(int server_fd) {
    struct sockaddr_in client_info;
    socklen_t client_info_len = sizeof(client_info);

    return accept(server_fd, (struct sockaddr *) &client_info, &client_info_len);
}

int write_socket(int client_sock, const char *buf) {
    return write(client_sock, buf, strlen(buf));
}

int read_socket(const int client_sock, char *buf, const int bytes) {
    return read(client_sock, buf, bytes);
}

int close_connection(const int sock) {
    printf("Closing connection %d\n", sock);
    shutdown(sock, SHUT_RDWR);
    close(sock);
    return 0;
}

int print_server_response(const int sock) {
    char buf[BUFFER_SIZE] = {0};
    const int bytes_read = read_socket(sock, buf, BUFFER_SIZE);
    if (bytes_read == -1) {
        perror("read");
        shutdown(sock, SHUT_RDWR);
        close(sock);
        return 1;
    }
    printf("Server sent: %s\n", buf);
    return 0;
}

void handle_server(int fd) {
    char buf[4096] = {0};
    proto_hdr_t *hdr = buf;
    read(fd, hdr, sizeof(proto_hdr_t) + sizeof(int));
    hdr->type = ntohl(hdr->type); // unpack the type
    hdr->len = ntohs(hdr->len);

    int *data = &hdr[1];
    *data = ntohl(*data); // protocol version one, packed

    if (*data != 1) {
        printf("Protocol mismatch!\n");
        return;
    }

    printf("Successfully connected to the server, protocol v1\n");
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <ip> [port=3412]\n", argv[0]);
        return 1;
    }
    int port;
    if (argc == 3) {
        port = atoi(argv[2]);
    } else {
        port = 3412;
    }

    struct sockaddr_in server_info = {0};
    server_info.sin_family = AF_INET;
    server_info.sin_addr.s_addr = inet_addr(argv[1]);
    server_info.sin_port = htons(port);

    const int client_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (client_sock == -1) {
        perror("socket");
        return 1;
    }
    int rc = 0;
    rc = connect(client_sock, (struct sockaddr *) &server_info, sizeof(server_info));
    if (rc == -1) {
        perror("connect");
        return 1;
    }

    // print_server_response(client_sock);
    handle_server(client_sock);
    close_connection(client_sock);
}
