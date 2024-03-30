#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <stdint.h>

#define SERVER_PORT 5432
#define BUF_SIZE 4096
#define TIMEOUT_SEC 2

// Define message types
#define FILE_REQUEST 0
#define ACK 1
#define FILE_INFO_AND_DATA 2
#define DATA 3
#define FILE_NOT_FOUND 4

// Define message structures
struct FileRequest {
    uint8_t type;
    uint8_t filename_size;
    char filename[];
};

struct Acknowledgment {
    uint8_t type;
    uint8_t num_sequences;
    uint16_t sequence_no[];
};

struct ServerMessage {
    uint8_t type;
    uint16_t sequence_number;
    uint8_t filename_size;
    char filename[256];
    uint32_t file_size;
    uint16_t block_size;
    char data[];
};

int main(int argc, char *argv[]) {
    struct sockaddr_in sin;
    struct sockaddr_storage client_addr;
    char clientIP[INET_ADDRSTRLEN]; /* For IPv4 addresses */
    socklen_t client_addr_len;
    char buf[BUF_SIZE];
    int len;
    int s;
    char *host;
    struct hostent *hp;

    FILE *fp;
    char *filename;

    struct timespec req;
    req.tv_sec = 0;
    req.tv_nsec = 10000;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s filename\n", argv[0]);
        exit(1);
    }
    filename = argv[1];

    if ((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("server: socket");
        exit(1);
    }

    memset((char *)&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
     sin.sin_addr.s_addr = INADDR_ANY;
  
  sin.sin_port = htons(SERVER_PORT);

    if ((bind(s, (struct sockaddr *)&sin, sizeof(sin))) < 0) {
        perror("server: bind");
        exit(1);
    } else {
        inet_ntop(AF_INET, &(sin.sin_addr), clientIP, INET_ADDRSTRLEN);
        printf("Server is listening at address %s : %d \n", clientIP, SERVER_PORT);
    }

    client_addr_len = sizeof(client_addr);

    while (recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr *)&client_addr, &client_addr_len)) {
        inet_ntop(client_addr.ss_family, &(((struct sockaddr_in *)&client_addr)->sin_addr), clientIP, INET_ADDRSTRLEN);
        printf("Weighting for request from client %s\n", clientIP);
        printf("Server got message from %s: %s [%d bytes]\n", clientIP, buf, len);

        // Process file request
        struct FileRequest *request = (struct FileRequest *)buf;
        if (request->type == FILE_REQUEST) {
            printf("Received file request for file: %s\n", request->filename);
            // Assuming the file is in the same directory as the server executable
            fp = fopen(request->filename, "rb");
            if (fp == NULL) {
                printf("File not found\n");
                // Send FILE_NOT_FOUND response
                struct FileRequest response;
                response.type = FILE_NOT_FOUND;
                response.filename_size = request->filename_size;
                strcpy(response.filename, request->filename);
                len = sizeof(struct FileRequest) + request->filename_size;
                sendto(s, &response, len, 0, (struct sockaddr *)&client_addr, client_addr_len);
            } else {
                printf("File found, sending file info and data\n");
                // Get file size
                fseek(fp, 0, SEEK_END);
                uint32_t file_size = ftell(fp);
                rewind(fp);

                // Send FILE_INFO_AND_DATA response
                struct ServerMessage response;
                response.type = FILE_INFO_AND_DATA;
                response.sequence_number = 0; // Assuming starting sequence number is 0
                response.filename_size = request->filename_size;
                strcpy(response.filename, request->filename);
                response.file_size = file_size;
                response.block_size = BUF_SIZE;
                int read_data_len;
                //memset(response.data, 0, BUF_SIZE);
                read_data_len =  fread(response.data, 1, BUF_SIZE, fp);
                printf("Server: Server Message is ready to send\n");

                // Read file data and send in blocks
                while (read_data_len > 0) {
                    len = sizeof(struct ServerMessage) + read_data_len;
                    sendto(s, &response, len, 0, (struct sockaddr *)&client_addr, client_addr_len);
                    printf("Server: Packet size %d\n", len);
                    printf("Server: Sent block %d\n", response.sequence_number);
                    // Wait for acknowledgment
                    struct Acknowledgment ack;
                    recvfrom(s, &ack, sizeof(struct Acknowledgment), 0, (struct sockaddr *)&client_addr, &client_addr_len);
                    printf("Server: Received first acknowledgment for block %d\n", ack.sequence_no[0]);
                    if (ack.type == ACK) {
                        printf("Acknowledgment received for sequence number %d\n", ack.sequence_no[0]);
                        // memset(response.data, 0, BUF_SIZE);
                        read_data_len =  fread(response.data, 1, BUF_SIZE, fp);
                        response.type = DATA;
                        response.sequence_number++;
                    } else {
                        printf("Invalid acknowledgment received\n");
                    }
                }

                fclose(fp);
            }
        } else {
            printf("Invalid message type received\n");
        }

        memset(buf, 0, sizeof(buf));
    }
}
