//
// Created by mdrozdov on 30.10.25.
//

#include "common.h"
#include "server.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>

#include <sys/socket.h>
#include <sys/poll.h>
#include <netinet/in.h>

#define MAX_CLIENTS 1024
#define BUFFER_SIZE 4096

typedef enum {
    NEW,
    CONNECTED,
    HELLO,
    MESSAGE,
    DISCONNECTED,
} client_state_e;

typedef struct {
    int sock;
    client_state_e state;
    char buf[BUFFER_SIZE];
} client_info_t;

static client_info_t clients[MAX_CLIENTS];
static int sfd = -1;

int start_server(const unsigned short port, const int backlog) {
    struct sockaddr_in server_info = {0};
    server_info.sin_addr.s_addr = 0;
    server_info.sin_family = AF_INET;
    server_info.sin_port = htons(port);

    sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd == -1) {
        perror("socket");
        return -1;
    }

    const int val = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

    if (bind(sfd, (struct sockaddr *) &server_info, sizeof(struct sockaddr_in)) == -1) {
        perror("bind");
        return -1;
    }

    if (listen(sfd, backlog) == -1) {
        perror("listen");
        return -1;
    }

    printf("[%d] Listening for incoming connections on port: %d\n", sfd, port);
    memset(&clients, 0, sizeof(client_info_t) * MAX_CLIENTS);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].sock = -1;
    }

    return sfd;
}

static int find_free_slot() {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].sock == -1) return i;
    }
    return -1;
}

static int find_client_slot(const int socket) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].sock == socket) return i;
    }
    return -1;
}

int accept_connections(const int server_fd) {
    struct sockaddr_in client_info;
    socklen_t client_info_len = sizeof(client_info);
    const int idx = find_free_slot();
    if (idx == -1) {
        printf("Max clients %d reached, not accepting more connections\n", MAX_CLIENTS);
        return -1;
    }

    const int sock = accept(server_fd, (struct sockaddr *) &client_info, &client_info_len);
    if (sock == -1) {
        perror("accept");
        return -1;
    }
    printf("[%d] New connection from %s:%d\n", sock, inet_ntoa(client_info.sin_addr), client_info.sin_port);

    client_info_t *info = &clients[idx];
    info->sock = sock;
    info->state = CONNECTED;

    return sock;
}

int write_socket(const int client_sock, const char *buf) {
    return (int) write(client_sock, buf, strlen(buf));
}

int close_connection(const int sock) {
    if (sock == -1) {
        printf("Invalid socket fd: %d\n", sock);
        return -1;
    }
    printf("Closing connection %d\n", sock);
    shutdown(sock, SHUT_RDWR);
    close(sock);
    return 0;
}

void handle_client(const int sock) {
    const char buf[4096] = {0};
    proto_hdr_t *hdr = buf;

    hdr->type = htonl(PROTO_HELLO); // pack the type
    hdr->len = sizeof(int);
    const unsigned int real_len = hdr->len;
    hdr->len = htons(hdr->len); // pack the len

    int *data = &hdr[1];
    *data = htonl(1); // protocol version one, packed
    write(sock, hdr, sizeof(proto_hdr_t) + real_len);
}

void handle_read(client_info_t *client) {
    printf("read");
    const size_t bytes_read = read(client->sock, client->buf, sizeof(client->buf));
    if (bytes_read <= 0) {
        printf("[%d] Client disconnected or erred\n", client->sock);
        close_connection(client->sock);
        client->sock = -1;
        client->state = DISCONNECTED;
        memset(client->buf, 0, sizeof(client->buf));
        return;
    }

    printf("[%d] Client sent: %s\n", client->sock, client->buf);
}

void handle_write(client_info_t *client) {
    printf("write");
    proto_hdr_t *hdr = client->buf;

    hdr->type = htonl(PROTO_HELLO); // pack the type
    hdr->len = sizeof(int);
    const unsigned int real_len = hdr->len;
    hdr->len = htons(hdr->len); // pack the len

    int *data = &hdr[1];
    *data = htonl(1); // protocol version one, packed
    const size_t bytes_written = write(client->sock, hdr, sizeof(proto_hdr_t) + real_len);
    if (bytes_written <= 0) {
        perror("write");
        close_connection(client->sock);
        client->sock = -1;
        client->state = DISCONNECTED;
        memset(client->buf, 0, sizeof(client->buf));
    }
}

void tick_server() {
    struct pollfd fds[MAX_CLIENTS];
    fds[0].fd = sfd;
    fds[0].events = POLLIN;

    int active_socks = 1;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        const client_info_t *client = &clients[i];
        if (client->sock != -1) {
            fds[active_socks].fd = client->sock;
            fds[active_socks].events = POLLIN | POLLOUT;
            active_socks++;
        }
    }

    const int ready_count = poll(fds, active_socks, -1);
    if (ready_count == -1) {
        perror("poll");
        return;
    }

    if (fds[0].revents & POLLIN) {
        accept_connections(sfd);
    }

    // Skip listening socket fds[0]
    for (int i = 1; i < active_socks; i++) {
        const int active_fd = fds[i].fd;
        const int client_slot = find_client_slot(active_fd);
        client_info_t *client = &clients[client_slot];
        if (fds[i].revents & POLLIN) {
            handle_read(client);
        }
        if (fds[i].revents & POLLOUT) {
            handle_write(client);
        }
    }
}

void stop_server() {
    if (sfd < 0) {
        printf("Server was not started, skip stopping\n");
        return;
    }

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].sock != -1) close_connection(clients[i].sock);
        memset(&clients[i], 0, sizeof(client_info_t));
    }
    close_connection(sfd);
}
