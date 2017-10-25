#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

// Returns the socket handle on successful connection, returns -1 on error
int setup_connection(const char *host, struct addrinfo *res) {
	int status, sockfd;
	struct addrinfo hints;
	status = getaddrinfo(host, "http", &hints, &res);
	if(status != 0) {
		fprintf(stderr, "getaddrinfo() failed\n");
		return -1;
	}

	sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if(sockfd < 0) {
		fprintf(stderr, "socket() failed\n");
		return -1;
	}

	status = connect(sockfd, res->ai_addr, res->ai_addrlen);
	if(status != 0) {
		fprintf(stderr, "connect() failed\n");
		return -1;
	}

	return sockfd;
}

void read_from_connection(int sockfd, char **buffer, ssize_t buf_size) {
	char msg[] = "GET /robots.txt HTTP/1.0\r\n\r\n";
	ssize_t sent = send(sockfd, msg, sizeof(msg), 0);
	if(sent < 0) {
		fprintf(stderr, "send() failed\n");
		exit(-1);
	}

	size_t nbytes = 0, rbytes = 0;
	char buf[buf_size];
	char *ptr = malloc(buf_size);
	do {
		rbytes = recv(sockfd, buf, buf_size, MSG_WAITALL);
		buf[buf_size] = '\0';
		nbytes += rbytes;
		ptr = realloc(ptr, nbytes);
		strcat(ptr, buf);
		printf("%s\n", ptr);
	} while(rbytes == buf_size);
}

int main(void) {
	struct addrinfo *res; // Storage for results of getaddrinfo
	int sockfd;
	const char hostname[] = "google.com";
	char *html;

	sockfd = setup_connection(hostname, res);
	if(sockfd > 0) 
		printf("Connection setup correctly!\n");
	else
		printf("Failed to connect to %s\n", hostname);


	read_from_connection(sockfd, &html, 512);
	printf("Read from the socket (%d): \n\n%s\n", strlen(html), html);
	free(html);

	// Closing socket and freeing the linked list in res
	freeaddrinfo(res); // Apparently this doesn't have to be freed.... IDK
	close(sockfd);
	return 0;
}