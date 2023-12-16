// Server program.

#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include<sys/wait.h>

#define UINT16_MAX 65535
#define UINT16_MIN 1

// Port Number Conversion
uint16_t strToUint(const char *strPortNum) {
  	char *end;
	unsigned long portNum = strtoul(strPortNum, &end, 10);
	if (*end != '\0' || portNum > UINT16_MAX || portNum < UINT16_MIN) {
		fprintf(stderr, "Invalid port number: %s\n", strPortNum);
		exit(1);
  	}
	return portNum;
}

// cmdline reminder: portnum, helper
int main(int argc, char *argv[]) {
	// TODO
	int serverfd;
	struct sockaddr_in servaddr;

	if (-1 == (serverfd = socket(AF_INET, SOCK_STREAM, 0))) {
		fprintf(stderr, "Server socket creation error...\n");
        return 1;
	}

	memset(&servaddr, 0, sizeof(struct sockaddr_in));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(strToUint(argv[1]));
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	// Bind with the server socket
	if (0 > (bind(serverfd, (struct sockaddr*)&servaddr, sizeof(struct sockaddr_in)))) {
		fprintf(stderr, "Bind error...\n");
		close(serverfd);
		return 1;
	}

	// Listening for client connections
	if (-1 == listen(serverfd, 3)) {
		fprintf(stderr, "Listening error...\n");
		return 1;
	}

	// Preventing zombie processes and ignoring other terminaion signals
	signal(SIGCHLD, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);

	long long serialNumber = 0;

	int clientfd;
	struct sockaddr_in cliaddr;
	memset(&cliaddr, 0, sizeof(struct sockaddr_in));

	pid_t cp;
	socklen_t sin_len;
	while (1) {
		sin_len = sizeof(struct sockaddr_in);
		if (0 > (clientfd = accept(serverfd, (struct sockaddr*)&cliaddr, &sin_len))) {
			fprintf(stderr, "Server accept error...\n");
			continue;
		}

		switch (cp = fork()) {
			// Child process
			case(0):
				close(serverfd);

				// Covert required data to string to pass on to helper
				char strSerialNumber[22];
				snprintf(strSerialNumber, sizeof(strSerialNumber), "%lld", serialNumber);
				char strClientfd[22];
				snprintf(strClientfd, sizeof(strClientfd), "%d", clientfd);

				if (execl(argv[2], argv[2], strClientfd, strSerialNumber, (char*)NULL) == -1) {
					fprintf(stderr, "Exec failed...\n");
					close(clientfd);
					exit(3);
				}

			// Error handling for fork
			case(-1):
				fprintf(stderr, "Forking error...\n");
				close(clientfd);
				exit(1);

			// Parent Process
			default:
				close(clientfd);
				serialNumber++;
				break;
		}
	}
	close(serverfd);
	return 0;
}
