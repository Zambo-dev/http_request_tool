#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
/* OpenSSL */
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509_vfy.h>

#define BUFFSIZE 1024

void errck()
{
	if (errno != 0)
	{
		printf("Error %d -> %s\n", errno, strerror(errno));
		errno = 0;
		exit(EXIT_FAILURE);
	}
}

int main(int argc, char **argv)
{
	int sockfd, result;
	char buff[BUFFSIZE];
	char *url = argv[1];
	char *ip = argv[2];
	char *port = argv[3];
	struct sockaddr_in host_info;
	SSL_CTX *ctx;
	SSL *ssl;

	/* Create a TLS client context with a CA certificate */
	ctx = SSL_CTX_new(TLS_client_method());
	result = SSL_CTX_use_certificate_file(ctx, "/cert.pem", SSL_FILETYPE_PEM);
	if(result != 1)
	{
		puts("ERROR: certificate missing!");
		return EXIT_FAILURE;
	}

	/* Create a socket and SSL session */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	errck();
	puts("Socket created!");

	/* Set the address and port of the server to connect to */
	host_info.sin_family = AF_INET;
	host_info.sin_port = htons(strtol(port, NULL, 10));
	host_info.sin_addr.s_addr = inet_addr(ip);

	/* Try to connect */
	result = connect(sockfd, (struct sockaddr *)&host_info, sizeof(host_info));
	errck();
	puts("Connected!");

	ssl = SSL_new(ctx);
	SSL_set_fd(ssl, sockfd);
	puts("Socket linked with SSL!");

	/* Run the OpenSSL handshake */
	result = SSL_connect(ssl);
	errck();

	/* Exchange some data if the connection succeeded */
	char *uri = strtok(url, "/");
	uri = strtok(NULL, "/");

	sprintf(buff, "GET /index.html HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", uri);
	printf("\nCLIENT > %s\n", buff);
	SSL_write(ssl, buff, strlen(buff) + 1);
	memset(buff, 0, BUFFSIZE);

	printf("SERVER >\n");
	while(SSL_read(ssl, buff, sizeof(buff)) > 0)
	{
		errck();
		printf("%s", buff);
		memset(buff, 0, BUFFSIZE);
	}

	/* Close connection */
	close(sockfd);
	SSL_free(ssl);
	SSL_CTX_free(ctx);

	return EXIT_SUCCESS;
}
