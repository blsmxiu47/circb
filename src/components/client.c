#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "../../include/client.h"

#define PORT 8080
#define MAX_MESSAGE_SIZE 1024

Client* init_client(const char* hostname, int port) {
	Client* client = malloc(sizeof(Client));
	// TODO: likely raise to caller
	if (!client) {
		perror("Failed to allocate memory for client");
		exit(EXIT_FAILURE);
	}
	
	// set client members
	// hostname, port come from arguments (specified by user)
	client->server_hostname = strdup(hostname);
	client->server_port = port;
	// init socket file descriptor to -1 as it is not yet connected
	client->socket_fd = -1;
	
	return client;
}

int connect_to_server(Client* client) {
	struct sockaddr_in server_address;
	
	// catch any errors creating a new socket for the client
	// breaking down the logic: client_fd first set to socket with the following params:
	//   * domain: AF_INET - IPv4 Internet protocols 
	//   * type: SOCK_STREAM - Provides sequenced, reliable, two-way, connection-based byte streams.  An out-of-band data transmission mechanism may be supported.
	//   * protocol: 0 - Normally only a single protocol exists to support a particular socket type within a given protocol family, in which case protocol can be specified as 0.
	// On error socket() will return -1, so the below addresses the case in which an error occurred.
	// On success, socket() will return a file descriptor for the new socket
	if ((client->socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Socket creation failed");
		return -1;
	}
	
	// Set sin_family of our serv_addr struct to AF_INET (see above)
	server_address.sin_family = AF_INET;
	// Network byte order should just refer to big-endian, which is the 
	// endianness used as standard by TCP/IP
	// Set sin_port to htons(PORT), so htons(8080) for example. The htons() 
	// function converts the unsigned short integer hostshort from host byte order 
	// (which could be big- or little-endian) to network byte order.
	server_address.sin_port = htons(client->server_port);
	// Below..
	//   * af:  AF_INET - IPv4 Internet protocols 
	//   * src: e.g. localhost, or whatever server_hostname member of client struct ends up being in this case
	//   * dst: ref to the sin_addr member from serv_addr struct. 
	//          Assuming this is the address for the server itself. So this is where the converted network address will go.
	if (inet_pton(AF_INET, client->server_hostname, &server_address.sin_addr) <= 0) {
		perror("Invalid address/Address not supported");
		return -1;
	}
	
	// Try initiating connection on socket. If error, print message.
	// int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
	//   * sockfd: client_fd - file descriptor of the socket in question
	//   * *addr: pointer to serv_addr, which contains the destination address and 
	//   port by this point (e.g. 127.0.0.1:8080, but it's a struct with the information 
	//   included in a sockaddr_in struct), and this is cast as sockaddr from sockaddr_in. 
	//   sockaddr is a generic descriptor for any kind of socket operation, whereas sockaddr_in 
	//   is a struct specific to IP-based communication (IIRC, "in" stands for "InterNet"). As far as 
	//   I know, this is a kind of "polymorphism" : the bind() function pretends to take a struct sockaddr *, 
	//   but in fact, it will assume that the appropriate type of structure is passed in; i. e. one that 
	//   corresponds to the type of socket you give it as the first argument."
	//   * addrlen: size of addr
	if (connect(client->socket_fd, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
		perror("Connection failed");
		return -1;
	}

	return 0;
}

// ssize_t send(int socket, const void *buffer, size_t length, int flags);
// Send the message
//   * socket: client_fd - again, file descriptor of the socket in question
//   * *buffer: hello - char* pointer to the buffer containing text that we want to send
//   * length: number of bytes in our char* hello
//   * flags: 0 - specifies the type of message transmission. There are a few possible flags, and 0 for none of them
// Returns the number of bytes sent if successful, or -1 if error	
int send_message(Client* client, const char* message) {
	return send(client->socket_fd, message, strlen(message), 0);
}

char* receive_message(Client* client) {
	// this buffer gets reused across calls
	static char buffer[1024];
	// void *memset(void s[.n], int c, size_t n);
	// The memset() function fills the first n bytes of the memory area
	// pointed to by s with the constant byte c.
	memset(buffer, 0, sizeof(buffer));
    
	// Similar to read()ing to buffer for now while using TCP communication 
	// in a blocking manner, but recv() makes it clearer we are working with 
	// sockets and also allows flags and flexibility in case we need it.
	// ssize_t recv(int sockfd, void buf[.len], size_t len, int flags);
	int bytes_received = recv(client->socket_fd, buffer, sizeof(buffer) - 1, 0);
	if (bytes_received < 0) {
		perror("Receive failed");
		return NULL;
	}

	return buffer;
}

// attempt to close the client socket and free memory as applicable
void close_client(Client* client) {
	// int close(int fd);
	// close() closes a file descriptor, so that it no longer refers to 
	// any file and may be reused.
	close(client->socket_fd);
	free(client->server_hostname);
	free(client);
}

int main (int argc, char* argv[]) {
	// TODO: not necessarily assuming command line args are given for the 
	// first time I think this will be okay but we should be prepped 
	// for the future when making this more flecible for other use cases
	const char* hostname = argv[1];
	// port as well from arguments, though this should also be parsed from config file no?
	int port = atoi(argv[2]);

	// init client - allocates memory and sets hostname, port, inits socket to -1 for now
	Client* client = init_client(hostname, port);
	printf("Server hostname: %s\n", client->server_hostname);
	printf("Server port: %d\n", client->server_port);
	printf("Client socket: %d\n", client->socket_fd);
	
	// try connecting to the server, return failure if non-zero code returned
	if (connect_to_server(client) != 0) {
		fprintf(stderr, "Failed to connect to server\n");
		close_client(client);
		return EXIT_FAILURE;
	}

	// Below, loop for enabling continuous read and write operations, i.e.
	// allowing users to send and receive messages freely
	// Using `poll()`... init fds with length of 2
	// The set of file descriptors to be monitored is specified in the 
	// fds argument, which is an array of structures of the following form:
	//   struct pollfd {
	//       int   fd;         /* file descriptor */
	//       short events;     /* requested events */
	//       short revents;    /* returned events */
	//   };
	struct pollfd fds[2];
	// standard input (user input)
	fds[0].fd = STDIN_FILENO;
	// data to read
	fds[0].events = POLLIN;
	// socket of connection to server
	fds[1].fd = client->socket_fd;
	// data to read
	fds[1].events = POLLIN;

	// set limit on message size
	char message[MAX_MESSAGE_SIZE];
	int should_run = 1;

	printf("Connected to server. Type /disconnect to exit.\n");

	while (should_run) {
		// -1 means no timeout, wait indefinitely (or rather, until told otherwise)
		// int poll(struct pollfd *fds, nfds_t nfds, int timeout);
		int poll_count = poll(fds, 2, -1);
		// peace out if poll() returns error
		if (poll_count == -1) {
			perror("poll");
			break;
		}

		// If there is user input (first file descriptor)
		if (fds[0].revents & POLLIN) {
			fgets(message, MAX_MESSAGE_SIZE, stdin);

			// Check if user wants to disconnect
			if (strncmp(message, "/disconnect", 11) == 0) {
				should_run = 0;
				break;
			}
			
			// actually try sending the message specified by user
			send_message(client, message);
		}

		// If there is activity from the server (via socket, second file descriptor)
		if (fds[1].revents & POLLIN) {
			// get response from client (using `recv()`)
			char* response = receive_message(client);
				if (response) {
					// if non-null, print so that user can see it
					printf("Server: %s\n", response);
				} else {
					// otherwise, notify that server is not connected via socket and break
					printf("Server disconnected.\n");
					should_run = 0;
					break;
				}
		}
	}
	// Sample communication for testing..
	// TODO: rm
	//send_message(client, "Hello, Server!");
	// TODO: failing in the recv() call here atm, returns null response
	// char* response = receive_message(client);
	// printf("Received from server: %s\n", response);
	// close_client(client);

	return EXIT_SUCCESS;
}
