//
// Created by mdrozdov on 30.10.25.
//

#ifndef ZERO2HERO_SERVER_H
#define ZERO2HERO_SERVER_H

typedef enum {
    PROTO_HELLO,
} proto_type_e;

typedef struct {
    proto_type_e type;
    unsigned int len;
} proto_hdr_t;

int start_server(unsigned short port, int backlog);

int accept_connections(int server_fd);

int write_socket(int client_sock, const char *buf);

int close_connection(int sock);

void handle_client(int client_sock);

void tick_server();

void stop_server();

#endif //ZERO2HERO_SERVER_H

