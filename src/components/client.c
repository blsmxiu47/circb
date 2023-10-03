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
	if ((client->socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        	perror("Socket creation failed");
        	return -1;
    	}

    	server_address.sin_family = AF_INET;
    	server_address.sin_port = htons(client->server_port);
    	// inet_pton returns 1 on success, 0 if `src` does not contain a character
	// string representing a valid network address in the specified address family,
	// and -1 if `af` does not contain a valid address family
	if (inet_pton(AF_INET, client->server_hostname, &server_address.sin_addr) <= 0) {
        	perror("Invalid address/Address not supported");
        	return -1;
    	}

	// try connecting client to server via our new socket
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
	// TODO: annotations
	memset(buffer, 0, sizeof(buffer));
    
	// TODO: annotations
    	int bytes_received = recv(client->socket_fd, buffer, sizeof(buffer) - 1, 0);
    	if (bytes_received < 0) {
        	perror("Receive failed");
        	return NULL;
    	}

	return buffer;
}

// TODO: annotations
void close_client(Client* client) {
	close(client->socket_fd);
	free(client->server_hostname);
	free(client);
}


// TODO: refactor this main() fn since we're gradually moving everything into the functions above
int main(int argc, char const* argv[])
{
	// initialize vars for connection status, the value that gets read from client file descriptor, and the client file descriptor tself
	int status, valread, client_fd;
	// struct for the server's internet address
	struct sockaddr_in serv_addr;
	// init message that we will be sending as a test
	char* hello = "Hello from client";
	// init buffer as an array with the first element as 0 and the remaining 1023 elements will implicitly be 0 as well
	char buffer[1024] = { 0 };
	// breaking down the logic: client_fd first set to socket with the following params:
	//   * domain: AF_INET - IPv4 Internet protocols 
	//   * type: SOCK_STREAM - Provides sequenced, reliable, two-way, connection-based byte streams.  An out-of-band data transmission mechanism may be supported.
	//   * protocol: 0 - Normally only a single protocol exists to support a particular socket type within a given protocol family, in which case protocol can be specified as 0.
	// On error socket() will return -1, so the below addresses the case in which an error occurred.
	// On success, socket() will return a file descriptor for the new socket
	// Not sure why it checks for < 0 as opposed to == -1. Is it faster, or maybe a standard?
	if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("\n Socket creation error \n");
		// return -1, not client_fd. Reason for this?
		return -1;
	}

	// Set sin_family of our serv_addr struct to AF_INET (see above)
	serv_addr.sin_family = AF_INET;
	// Set sin_port to htons(PORT), so htons(8080) for example. The htons() function converts the unsigned short integer hostshort from host byte order (which could be big- or little-endian) to network byte order.
	// Network byte order should just refer to big-endian, which is the endianness used as standard by TCP/IP
	serv_addr.sin_port = htons(PORT);

	// int inet_pton(int af, const char *restrict src, void *restrict dst): This function converts the character string src into a network 
	// address structure in the address family specified in first arg `af`, then copies the network address 
	// structure to dst (destination). The af argument must be either AF_INET or AF_INET6 (because that is what happens to be currently supported. not sure why). dst is written in network byte order. 
	// 
	// Below,,,
	//   * af: AF_INET - IPv4 Internet protocols 
	//   * src: localhost
	//   * dst: ref to the sin_addr member from serv_addr struct. Assuming this is the address for the server itself. So this is where the converted network address will go.
	//   Again in this case, if inet_pton() returns -1, then print the error message
	if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)
		<= 0) {
		printf(
			"\nInvalid address/ Address not supported \n");
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
	//   * addrlen: size of addr. (Why is this needed? Do you ever set this to anything other than the size of the prev arg? Probably only in cases where for whatever reason the second arg for address is more than just the address, like a struct containing address struct as a member inside it, with additional members that should not be counted in sizeof, though atm not sure of a good example of this)
	if ((status
		= connect(client_fd, (struct sockaddr*)&serv_addr,
				sizeof(serv_addr)))
		< 0) {
		printf("\nConnection Failed \n");
		return -1;
	}
	// ssize_t send(int socket, const void *buffer, size_t length, int flags);
	// Send the message
	//   * socket: client_fd - again, file descriptor of the socket in question
	//   * *buffer: hello - char* pointer to the buffer containing text that we want to send
	//   * length: number of bytes in our char* hello
	//   * flags: 0 - specifies the type of message transmission. There are a few possible flags, and 0 for none of them
	// Returns the number of bytes sent if successful, or -1 if error
	send(client_fd, hello, strlen(hello), 0);
	printf("Client sent: %s\n", hello);
	// ssize_t read(int fd, void buf[.count], size_t count);
	// read() attempts to read up to count bytes from file descriptor fd into the buffer starting at buf.
	//   * fd: client_fd - again, file descriptor of the socket in question
	//   * buf[.count]: buffer - our array of 1024 0s at this point
	//   * count: 1024, maximum number of bytes to read. This is the length of buffer in this case
	valread = read(client_fd, buffer, 1024);
	// print out the contents of buffer
	printf("Client Received: %s\n", buffer);

	// closing the connected socket
	// int close(int fd);
	// returns 0 on success and -1 on error (e.g. invalid fd)
	close(client_fd);
	printf("Client socket closed\n");
	return 0;
}

