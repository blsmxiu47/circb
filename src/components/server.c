// Sample TCP Server
#include <netinet/in.h>
#include <poll.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "../../include/server.h"

#define PORT 8080
#define MAX_CLIENTS 100 // TODO: Likely update this later

void handle_client(int client_fd) {
	char buffer[1024];
	// recv() to take in messages from a client, mirrors the method used in the client script
	int bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

	if (bytes_read <= 0) {
		// Client disconnected or error
		close(client_fd);
		return;
	}

	// Null terminate the received data
	buffer[bytes_read] = '\0';
	printf("Received: %s\n", buffer);

    	// TODO: add logic to process the message. I believe this is 
	// ultimately where the magic will happen in terms of proccessing 
	// commands that the user through the client is sending to the server
    	// For now, simply just echoing back the message for the sake
	// of testing
    	send(client_fd, buffer, bytes_read, 0);
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


// TODO: move this elsewhere and eventually delete once decisions have been made
// NOTES on threading/IO: (but currently using poll at the moment)
	// TODO: I think we're going to be using poll or epoll (and if not, then select. But 
	// select is limited for modern apps due to 1024 (or 1023?) fds per server.. Check out 
	// the theoretical O() complexity of each. poll options just seem superior to select(). 
	// epoll() has additional customization options and functionality over poll(), though is 
	// linux-specific. epoll() is also technically a group of 3 APIs as oppoosed to just 1 
	// each for select() and poll(). epoll() also appears to simply have efficiency 
	// improvements over poll(). The above all really may not matter for my use case, but 
	// probably better to practice with tools we'd use in real life)


void start_server(Server* server) {
	// +1 for the main listening socket
	struct pollfd fds[MAX_CLIENTS + 1];
	// Start with the main socket
    	int num_fds = 1;

    	// File descriptor of main listening socket for client connection (index 0)
	fds[0].fd = server->socket_fd;
	// data to read
    	fds[0].events = POLLIN;

	// loop to handle continuous handling of clients 
	// (in this case the only limit on the number of 
	// clients is hardware resources and the limit 
	// set in MAX_CLIENTS
    	while (1) {
		// third arg -1 for no timeout
        	int poll_count = poll(fds, num_fds, -1);
        	if (poll_count == -1) {
			perror("poll");
			break;
		}

		// Looping over all the file descriptors, up to the limit 
		// specified in MAX_CLIENTS (+1 for the main listening socket)
		for (int i = 0; i < num_fds; i++) {
			// if there is input to be had associated with this file descriptor 
			if (fds[i].revents & POLLIN) {
				// if the current file descriptor matches the connection to client
				// AKA if listening on this socket?
				if (fds[i].fd == server->socket_fd) {
					// New client connection
					struct sockaddr_in client_address;
					socklen_t client_len = sizeof(client_address);
					// try to begin accepting connections on this socket
					int new_client_fd = accept(server->socket_fd, (struct sockaddr*)&client_address, &client_len);
					if (new_client_fd == -1) {
						perror("accept");
						continue;
					}
					// refuse connections if this server is hit the limit specified by MAX_CLIENTS
					if (num_fds >= MAX_CLIENTS) {
						fprintf(stderr, "Too many clients. Connection refused.\n");
						close(new_client_fd);
						continue;
					}

					// assuming connection is not refused, set the file descriptor of 
					// the current item in `fds` to the newly created connection
					fds[num_fds].fd = new_client_fd;
					// and set the events member to POLLIN (readiness to read)
                    			fds[num_fds].events = POLLIN;
					// increment the counter for fds
                    			num_fds++;
				} else {
					// case when existing client sent a message
                    			handle_client(fds[i].fd);
				}
			}
		}
	}

   //int opt = 1;

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
    }
}

void stop_server(Server* server) {
    // TODO: implement the logic to shut down the server gracefully.
    // This includes closing any open sockets, freeing allocated memory, etc.
    // The free() function shall cause the space pointed to by ptr to be 
    // deallocated; that is, made available for further allocation. 
    // If ptr is a null pointer, no action shall occur.
    // So, ensures there is memory space for sock in advance
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
    // stop_server(server);

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

