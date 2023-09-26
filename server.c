// Sample TCP Server
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#define PORT 8080

int main(int argc, char const* argv[])
{
	// init vars for server file descriptor, new socket to accept connections (created from but will be separate from server_fd), 
	// and var to store the message we try reading in
	int server_fd, new_socket, valread;
	// init IPv4 address for server socket
	struct sockaddr_in address;
	// Option value for 
	int opt = 1;
	int addrlen = sizeof(address);
	char buffer[1024] = { 0 };
	char* hello = "Hello from server";

	// CREATE
	// Creating socket file descriptor
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	// Forcefully attaching socket to the port 8080
	// int setsockopt(int socket, int level, int option_name, const void *option_value, socklen_t option_len);
	//   * socket: server_fd - file descriptor of our server socket
	//   * level: SOL_SOCKET - set connections at the socket level
	//   * option_name: SO_REUSEADDR | SO_REUSEPORT - option that we want to set
	//   * *option_value: &opt - in this case it is 1. So allow reuse of addr or(?) port
	//   * option_len: sizeof(opt) - length of opt. Still dont understand why these len args are always required
	// setsocketopt() should return 0 on success, -1 on failure
	// Attempting to set multiple options in one line here. Check back if issues arise
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}

	address.sin_family = AF_INET;
	// set address of address's address to INADDR_ANY, or bind to all available interfaces, not specific one
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);

	// BIND
	// Forcefully attaching socket to the port 8080
	// bind() assigns the address specified by addr to the socket referred to by the file descriptor sockfd
	// int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
	// ((see client.c for arg annotations))
	if (bind(server_fd, (struct sockaddr*)&address,
			sizeof(address))
		< 0) {
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
	// int listen(int sockfd, int backlog);
	//   * backlog: 3 - The backlog argument defines the maximum length to which the queue of pending connections for sockfd may grow
	if (listen(server_fd, 3) < 0) {
		perror("listen");
		exit(EXIT_FAILURE);
	} else {
		printf("Server is listening for connections\n");
	}
	// Set up new_socket to accept connections
	// int accept(int sockfd, struct sockaddr,*_Nullable restrict addr, socklen_t *_Nullable restrict addrlen);
	// accept() extracts the first connection request on the queue of pending connections for the listening socket, sockfd,
	// creates a new connected socket, and returns a new file descriptor referring to that socket.  The newly created socket 
	// is not in the listening state.  The original socket sockfd is unaffected by this call.
	if ((new_socket
		= accept(server_fd, (struct sockaddr*)&address,
				(socklen_t*)&addrlen))
		< 0) {
		perror("accept");
		exit(EXIT_FAILURE);
	} else {
		printf("Server is ready to accept connections\n");
	}
	// Try reading from the new socket accepting connections to buffer
	valread = read(new_socket, buffer, 1024);
	printf("Server received: %s\n", buffer);
	// Echo back the received message
	send(new_socket, buffer, strlen(buffer), 0);
	printf("Server sent: %s\n", buffer);

	// closing the connected socket
	close(new_socket);
	printf("Server socket closed\n");
	// closing the listening socket
	shutdown(server_fd, SHUT_RDWR);
	printf("Server shutdown\n");
	return 0;
}

