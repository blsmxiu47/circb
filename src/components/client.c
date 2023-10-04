#include "../../include/client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#define PORT 8080

Client* init_client(const char* hostname, int port) {
    	// allocate memory for client object
	Client* client = malloc(sizeof(Client));
	// TODO: likely raise to caller
    	if (!client) {
        	perror("Failed to allocate memory for client");
        	exit(EXIT_FAILURE);
    	}
	
	// set client members
	// hostname comes from argument (specified by user originially)
    	client->server_hostname = strdup(hostname);
	// same with port
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
	// In terms of why it checks for < 0 as opposed to == -1.. appears to be 
	// 1. a standard, and 2. technically safer as a general practice since this would 
	// catch cases where some other error code is returned (though not the case here with socket()
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
	// Below,,,
	//   * af: AF_INET - IPv4 Internet protocols 
	//   * src: e.g. localhost, or whatever server_hostname member of client struct ends up being in this case
	//   * dst: ref to the sin_addr member from serv_addr struct. Assuming this is the address for the server itself. So this is where the converted network address will go.
	//   Again in this case, if inet_pton() returns -1, then print the error message
    	// inet_pton returns 1 on success, 0 if `src` does not contain a character
	// string representing a valid network address in the specified address family,
	// and -1 if `af` does not contain a valid address family
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
	//   * addrlen: size of addr. (Why is this needed? Do you ever set this to anything other than the 
	//   size of the prev arg? Probably only in cases where for whatever reason the second arg for 
	//   address is more than just the address, like a struct containing address struct as a member 
	//   inside it, with additional members that should not be counted in sizeof, though atm not 
	//   sure of a good example of this)
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

int main (int argc, char const* argv[]) {
	// TODO: not necessarily assuming command line args are given for the 
	// foirst time I think this will be okay but we should be prepped 
	// for the future when making this more flecible for other use cases
	const char* hostname = argv[1];
	// port as well from arguments, though this should also be parsed from config file no?
	int port = atoi(argv[2]);

	// init client - allocates memory and sets hostname, port, inits socket to -1 for now
	Client* client = init_client(hostname, port);

	// try connecting to the server, return failure if non-zero code returned
	if (connect_to_server(client) != 0) {
		fprintf(stderr, "Failed to connect to server\n");
		close_client(client);
		return EXIT_FAILURE;
	}

	// Sample communication for testing..
	// TODO: rm
	// send_message(client, "Hello, Server!");
	// char* response = receive_message(client);
    	// printf("Received from server: %s\n", response);
	
	close_client(client);

	return EXIT_SUCCESS;
}
