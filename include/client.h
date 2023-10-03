#ifndef CLIENT_H
#define CLIENT_H

typedef struct {
    int socket_fd;
    char* server_hostname;
    int server_port;
    // TODO: additional client-specific data.
} Client;

Client* init_client(const char* hostname, int port);
int connect_to_server(Client* client);
int send_message(Client* client, const char* message);
char* receive_message(Client* client);
void close_client(Client* client);

#endif
