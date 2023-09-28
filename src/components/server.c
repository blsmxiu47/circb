// Sample TCP Server
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "../../include/server.h"

#define PORT 8080
#define MAX_CLIENTS 64

void *handle_client(void *client_sock) {
    int sock = *((int *)client_sock);
    // The free() function shall cause the space pointed to by ptr to be 
    // deallocated; that is, made available for further allocation. 
    // If ptr is a null pointer, no action shall occur.
    // So, ensures there is memory space for sock in advance
    free(client_sock);

    // TODO: read and write to the client socket, and implement IRC protocol.

    close(sock);
    // from p_thread.h
    // The pthread_exit() function terminates the calling thread and 
    // returns a value via retval (NULL here) that (if the thread is joinable) is 
    // available to another thread in the same process that calls pthread_join(3).
    // TODO: for the sake of learning, understand what this is doing more thoroughly
    pthread_exit(NULL);
}

// from server.h
Server* init_server(char* hostname, int port) {
    Server *server = malloc(sizeof(Server));
    // malloc returns a pointer to the block of memory that has been allocated for dynamic memory 
    // usage (as opposed to static or automatic)
    // With dynamic memory usage, when the memory is no longer needed, the pointer 
    // is passed to free which deallocates the memory so that it can be used for other purposes. 
    // Using dynamic memory allocation it is possible to fail to allocate memory,
    // for instance if there is simply not enough memory available
    // TODO: handle this issue more tactfully I assume.
    if (!server) {
        perror("Failed to allocate memory for Server");
        exit(EXIT_FAILURE);
    }
    // The strdup() function returns a pointer to a new string which is a duplicate of the 
    // string s.  Memory for the new string is obtained with malloc(3), and can 
    // be freed with free(3).
    // Essentially, strdup() does 2 things: 1. allocates enough memory to hold the contents
    // of the string pointed to by hostname, including the null terminator, and 2. copies 
    // the contents of the string hostname into this newly allocated memory block.
    // So, the overall effect of server->hostname = strdup(hostname); is that the 
    // hostname string is duplicated in memory, and the Server structure now holds a
    // pointer to this duplicate, allowing the server structure to safely hold and 
    // manage its own copy of the hostname.
    server->hostname = strdup(hostname); // Remember to free this during server shutdown.
    // And port is simply assigned to the port member of server
    server->port = port;
    // Initialize other server state information here.
    // TODO: tktk
    return server;
}


void start_server(Server* server) {
    // init var for server file descriptor
    int server_fd;
    // init IPv4 address for server socket
    struct sockaddr_in address;
    // Argument for bind(), accept()
    int addrlen = sizeof(address);
    // options
    int opt = 1;

    // CREATE
    // Creating socket file descriptor...
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	perror("socket failed");
	exit(EXIT_FAILURE);
    }

    // int setsockopt(int socket, int level, int option_name, const void *option_value, socklen_t option_len);
    //   * socket: server_fd - file descriptor of our server socket
    //   * level: SOL_SOCKET - set connections at the socket level
    //   * option_name: SO_REUSEADDR | SO_REUSEPORT - option that we want to set
    //   * *option_value: &opt - in this case it is 1. So allow reuse of addr or(?) port
    //   * option_len: sizeof(opt) - length of opt. Still dont understand why these len args are always required
    // setsocketopt() should return 0 on success, -1 on failure
    // Attempting to set multiple options in one line here. Check back if issues arise
    // if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
    //	   perror("setsockopt");
    //	   exit(EXIT_FAILURE);
    // }
    // if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
    //	   perror("setsockopt");
    //	   exit(EXIT_FAILURE);
    // }

    address.sin_family = AF_INET;
    // set address of address's address to INADDR_ANY, or bind to all available interfaces, not specific one
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(server->port);

    // BIND
    // Forcefully attaching socket to the port 8080
    // bind() assigns the address specified by addr to the socket referred to by the file descriptor sockfd
    // int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
    // ((see client.c for arg annotations))
    if (bind(server_fd, (struct sockaddr*)&address, addrlen) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    // int listen(int sockfd, int backlog);
    //   * backlog: 10 - The backlog argument defines the maximum length to which the queue of pending connections for sockfd may grow
    if (listen(server_fd, 10) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    printf("Server is listening on port %d...\n", server->port);

    // Set up new_socket to accept connections
    // int accept(int sockfd, struct sockaddr,*_Nullable restrict addr, socklen_t *_Nullable restrict addrlen);
    // accept() extracts the first connection request on the queue of pending connections for the listening socket, sockfd,
    // creates a new connected socket, and returns a new file descriptor referring to that socket.  The newly created socket 
    // is not in the listening state.  The original socket sockfd is unaffected by this call.
    while (1) {
	// new socket to accept connections (created from but will be separate from server_fd)
        int *new_sock = malloc(sizeof(int));
        if ((*new_sock = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            continue;
        }
	// TODO: Annotations
        pthread_t thread_id;
	// TODO: Annotations
        if (pthread_create(&thread_id, NULL, handle_client, (void*) new_sock) < 0) {
            perror("could not create thread");
            continue;
        }
    }
}

void stop_server(Server* server) {
    // TODO: implement the logic to shut down the server gracefully.
    // This includes closing any open sockets, freeing allocated memory, etc.
    free(server->hostname);
    // ... Free other resources.
    free(server);
}

int main() {
    char *hostname = "127.0.0.1";
    int port = 8080;
    Server* server = init_server(hostname, port);
    start_server(server);
    // TODO: Handle signals to stop the server gracefully.
    // TODO: also remember freeing memory
    // free(server->hostname);
    // free(server);
    return 0;
}


// int main(int argc, char const* argv[])
// {
	// char buffer[1024] = { 0 };
	// char* hello = "Hello from server";

	// Try reading from the new socket accepting connections to buffer
	// valread = read(new_socket, buffer, 1024);
	// printf("Server received: %s\n", buffer);
	// Echo back the received message
	// send(new_socket, buffer, strlen(buffer), 0);
	// printf("Server sent: %s\n", buffer);

	// closing the connected socket
	// close(new_socket);
	// printf("Server socket closed\n");
	// closing the listening socket
	// shutdown(server_fd, SHUT_RDWR);
	// printf("Server shutdown\n");
	// return 0;
// }

