// Client program.

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

#define UINT16_MAX 65535
#define UINT16_MIN 1

#define _FILE_OFFSET_BITS 64

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

// Writes to the socket of the server
void transferData(int toServerfd, const char* data, size_t length) {
    ssize_t totalWrite = 0, bytesWriten = 0;
    while (totalWrite < length) {
        bytesWriten = write(toServerfd, data + totalWrite, length - totalWrite);
        if (bytesWriten < 0) {
            fprintf(stderr, "Client: Error sending data...\n");
            close(toServerfd);
            exit(2);
        } else if (bytesWriten == 0) {
            fprintf(stderr, "Client: EOF occured...\n");
            close(toServerfd);
            exit(1);
        }
        totalWrite += bytesWriten;
    }
}

// cmdline reminder: IP-address, portnum, username, filename
int main(int argc, char *argv[]) {
	// TODO
	if (argc < 5) {
		fprintf(stderr, "Provide all required cmd arguments...\n");
		exit(1);
	}

	int clientfd;
	struct sockaddr_in servaddr;

	// Assigning the internet address
	memset(&servaddr, 0, sizeof(struct sockaddr_in));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(strToUint(argv[2])); // setting the port number

	// Validating the IPv4 address and setting it
	if (inet_pton(AF_INET, argv[1], &(servaddr.sin_addr)) != 1) {
		fprintf(stderr, "Invalid IPv4 address: %s\n", argv[1]);
		exit(1);
	}

    // Creating a socket and connecting to the server
    if (-1 == (clientfd = socket(AF_INET, SOCK_STREAM, 0))) {
        fprintf(stderr, "Socket creation error...\n");
        exit(1);
    }

    if (-1 == (connect(clientfd, (struct sockaddr *)&servaddr, sizeof(struct sockaddr_in)))) {
        fprintf(stderr, "Connecting error...\n");
        exit(1);
    }

    // 1. Send username
    transferData(clientfd, argv[3], strlen(argv[3]));
    transferData(clientfd, "\n", 1);

    // 2. Send file name
    transferData(clientfd, argv[4], strlen(argv[4]));
    transferData(clientfd, "\n", 1);

    FILE *file = fopen(argv[4], "rb");
    if (file == NULL) {
        fprintf(stderr, "Opening file error...\n");
        exit(1);
    }
    if (fseeko(file, 0L, SEEK_END) == -1) {
        fprintf(stderr, "Error seeking file cursor to end...\n");
        exit(1);
    }
    // Stores the size of the file
    long long size = (long long)ftello(file);

    // 3. Send file size
    char strFileSize[12] = {0};
    snprintf(strFileSize, sizeof(strFileSize), "%lld", size);
    transferData(clientfd, strFileSize, strlen(strFileSize));
    transferData(clientfd, "\n", 1);


    if (fseeko(file, 0L, SEEK_SET) == -1) {
        fprintf(stderr, "Error seeking file cursor to start...\n");
        exit(1);
    }

    // 4. Send the file content
    char content[2] = {0};
    size_t contentRead = 0;
    while (1) {
        contentRead = fread(content, sizeof(char), 1, file);
        if (contentRead < 0) {
            fprintf(stderr, "Error reading from file...\n");
            fclose(file);
            close(clientfd);
            exit(1);
        } else if (contentRead == 0) {
            break;
        } else {
            if (write(clientfd, content, 1) < 0) {
                close(clientfd);
                fclose(file);
                exit(1);
            }
        }
    }
    if (fseeko(file, 0L, SEEK_SET) == -1) exit(2);
    fclose(file);
    
    // Recieve Data from server
    char strSerialNumber[12] = {0};
    char character;
    ssize_t totalRead = 0, readSocket = 0;
    while (totalRead < 11) {
        readSocket = read(clientfd, &character, 1);
        if (readSocket < 0) {
            fprintf(stderr, "Error reading the serial number from server...\n");
            close(clientfd);
            exit(1);
        } else if (readSocket == 0) {
            fprintf(stderr, "EOF while reading the serial number from server...\n");
            close(clientfd);
            exit(3);
        }

        if (character == '\n') {
            strSerialNumber[totalRead] = character;
            break;
        } else {
            strSerialNumber[totalRead++] = character;
        }
    }

    if (strSerialNumber[totalRead] != '\n') {
        fprintf(stderr, "Server bug: Serial Number is not followed by a newline character...\n");
        close(clientfd);
        exit(4);
    }

    // Check for non-digit
    for (int i = 0; i < totalRead-1; i++) {
        if ((strSerialNumber[i] < '0' || strSerialNumber[i] > '9')) {
            fprintf(stderr, "Server bug: Serial Number contains non-digit characters...\n");
            close(clientfd);
            exit(4);
        }
    }

    printf("%s", strSerialNumber);
    close(clientfd);

  return 0;
}
