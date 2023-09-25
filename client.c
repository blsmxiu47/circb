// Sample Client
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#define PORT 8080

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

	// Convert IPv4 and IPv6 addresses from text to binary form.
	// int inet_pton(int af, const char *restrict src, void *restrict dst): This function converts the character string src into a network 
	// address structure in the address family specified in first arg `af`, then copies the network address 
	// structure to dst (destination). The af argument must be either AF_INET or AF_INET6 (because that is what happens to be currently supported. not sure why). dst is written in network byte order. 
	// 
	// Below,,,
	//   * af: AF_INET - IPv4 Internet protocols 
	//   * src: localhost
	//   * dst: ref to whatever the sin_addr from serv_addr struct is. Assuming this is the address for the server itself
	//   Again in this case, if inet_pton() returns -1, then print the error message
	if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)
		<= 0) {
		printf(
			"\nInvalid address/ Address not supported \n");
		return -1;
	}

	if ((status
		= connect(client_fd, (struct sockaddr*)&serv_addr,
				sizeof(serv_addr)))
		< 0) {
		printf("\nConnection Failed \n");
		return -1;
	}
	send(client_fd, hello, strlen(hello), 0);
	printf("Hello message sent\n");
	valread = read(client_fd, buffer, 1024);
	printf("%s\n", buffer);

	// closing the connected socket
	close(client_fd);
	return 0;
}

