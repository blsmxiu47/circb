typedef struct server {
    char* hostname;
    int port;
    int socket_fd;
    // TODO: Other state information like connected clients, channels, etc.
} Server;

Server* init_server(char* hostname, int port);
void start_server(Server* server);
void stop_server(Server* server);
