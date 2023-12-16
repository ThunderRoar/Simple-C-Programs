// Optional helper program for server-side child process.

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

// Writes to the socket of the server
void transferSN(int clientfd, const char* data, ssize_t length) {
    if (length != write(clientfd, data, length)) {
        fprintf(stderr, "Server: Error sending data...\n");
        _exit(2);
    }
}

// Send HDERR\n to client
void sendHderr(int cfd) {
    transferSN(cfd, "HDERR", 5);
    transferSN(cfd, "\n", 1);
}

// Reads the client data till newline
void readTillNL(int clientfd, char *buf, ssize_t len) {
    ssize_t totalRead = 0, sizeRead = 0;
    char character;
    while (totalRead < len-1) {
        sizeRead = read(clientfd, &character, 1);
        if (sizeRead < 0) {
            fprintf(stderr, "Error while reading data from client...\n");
            close(clientfd);
            _exit(1);
        } else if (sizeRead == 0) {
            fprintf(stderr, "EOF while reading data from client...\n");
            close(clientfd);
            _exit(1);
        }

        if (character != '\n') {
            buf[totalRead++] = character;
        } else {
            buf[totalRead] = '\0';
            return;
        }
    }
    
    fprintf(stderr, "Overflow in reading data...\n");
    sendHderr(clientfd);
    close(clientfd);
    _exit(1);
}


void checkUserName(int clientfd, char *userName, ssize_t unSize) {

    readTillNL(clientfd, userName, unSize);
    ssize_t unLen = strlen(userName);
    
    for (int i = 0; i < unLen; i++) {
        if (!( (userName[i] >= '0' && userName[i] <= '9') || 
               (userName[i] >= 'a' && userName[i] <= 'z') || 
               (userName[i] >= 'A' && userName[i] <= 'Z') ) ) {
                fprintf(stderr, "Username contains non alphanumeric characters...\n");
                sendHderr(clientfd);
                close(clientfd);
                _exit(3);
             }
    }
}

void checkFileName(int clientfd, char *fileName, ssize_t fnSize) {

    readTillNL(clientfd, fileName, fnSize);
    ssize_t fnLen = strlen(fileName);

    for (int i = 0; i < fnLen; i++) {
        if (fileName[i] == '/' || fileName[i] == '\n') {
            fprintf(stderr, "Invalid file name: Contains '/' or newline...\n");
            sendHderr(clientfd);
            close(clientfd);
            _exit(4);
        }
    }
}

void checkFileSize(int clientfd, char *fileSize, ssize_t fsSize) {

    readTillNL(clientfd, fileSize, fsSize);
    ssize_t fsLen = strlen(fileSize);

    for (int i = 0; i < fsLen; i++) {
        if (fileSize[i] < '0' || fileSize[i] > '9') {
            fprintf(stderr, "Invalid file size...\n");
            sendHderr(clientfd);
            close(clientfd);
            _exit(5);
        }
    }
}

void readFileContent(int clientfd, ssize_t fileContentSize, char *saveFileName) {
    FILE *file = fopen(saveFileName, "wb");
    if (file == NULL) {
        fprintf(stderr, "File opening error...\n");
        close(clientfd);
        exit(6);
    }

    char data;
    ssize_t fileRead = 0;

    for (unsigned long reading = 0; reading < fileContentSize; reading++) {
        fileRead = read(clientfd, &data, 1);
        if (fileRead == -1) {
            perror("Error reading from client");
            fclose(file);
            close(clientfd);
            _exit(6);
        } else if (fileRead == 0) {
            !(remove(saveFileName)) ? fprintf(stderr, "Premature EOF: File deleted...\n") : fprintf(stderr, "Error deleting file...\n");
            fclose(file);
            close(clientfd);
            _exit(6);
        }
        
        if (fwrite(&data, sizeof(char), 1, file) < fileRead) {
            fprintf(stderr, "Error writing to the file...\n");
            fclose(file);
            close(clientfd);
            _exit(6);
        }
    }
    fclose(file);
}

// argv[1] = strClientfd, argv[2] = strNumClients
int main(int argc, char *argv[]) {
    // Optional TODO

    char *err;
    int clientfd = strtol(argv[1], &err, 10);
    if (*err != '\0') {
        fprintf(stderr, "Error converting the client fd to int...\n");
        _exit(7);
    }
    
    // Check username from client
    char userName[10] = {0};
    checkUserName(clientfd, userName, sizeof(userName));

    // Check file name from client
    char fileName[102] = {0};
    checkFileName(clientfd, fileName, sizeof(fileName));

    // Check file size from client for reading the file content
    char fileSize[12] = {0};
    checkFileSize(clientfd, fileSize, sizeof(fileSize));

    // Convert string file size into long
    char *conversionErr;
    long fs = strtol(fileSize, &conversionErr, 10);
    if (*conversionErr != '\0') {
        fprintf(stderr, "Error converting the file size to int...\n");
        _exit(8);
    }

    char saveFile[120] = {0};
    snprintf(saveFile, sizeof(saveFile), "%s-%s-%s", userName, argv[2], fileName);

    // Reading file content after validations
    readFileContent(clientfd, fs, saveFile);

    // Sending the serial number after the file saving and validating the client sent info
    ssize_t totalWrite = 0, bytesWriten = 0;
    while (totalWrite < strlen(argv[2])) {
        bytesWriten = write(clientfd, argv[2] + totalWrite, strlen(argv[2]) - totalWrite);
        if (bytesWriten < 0) {
            fprintf(stderr, "Server: Error sending data...\n");
            close(clientfd);
            exit(2);
        } else if (bytesWriten == 0) {
            fprintf(stderr, "Server: EOF occured...\n");
            close(clientfd);
            exit(1);
        }
        totalWrite += bytesWriten;
    }

    transferSN(clientfd, "\n", 1);

    return 0;
}
