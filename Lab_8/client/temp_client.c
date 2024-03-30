#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
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
    char filename[256];
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
    FILE *fp;
    struct hostent *hp;
    struct sockaddr_in sin;
    char *host;
    char buf[BUF_SIZE];
    int s;
    int len;
    socklen_t server_addr_len;

    if ((argc != 2) && (argc != 3)) {
        fprintf(stderr, "usage: client serverIP [download_filename(optional)]\n");
        exit(1);
    }

    host = argv[1];

    if (argc == 3) {
        fp = fopen(argv[2], "wb");
        if (fp == NULL) {
            fprintf(stderr, "Error opening output file\n");
            exit(1);
        }
    }

    hp = gethostbyname(host);
    if (!hp) {
        fprintf(stderr, "client: unknown host: %s\n", host);
        exit(1);
    } else
        printf("Host %s found!\n", argv[1]);

    memset((char *)&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    memcpy((char *)&sin.sin_addr, hp->h_addr, hp->h_length);
    sin.sin_port = htons(SERVER_PORT);

    if ((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("client: socket");
        exit(1);
    }

    printf("Client will get data from to %s:%d.\n", argv[1], SERVER_PORT);
    printf("To play the music, pipe the download file to a player, e.g., ALSA, SOX, VLC: cat recvd_file.wav | vlc -\n");

    // Prepare and send file request
    struct FileRequest *request;
    int filename_size = strlen(argv[2]);
    request = malloc(sizeof(struct FileRequest) + filename_size);
    request->type = FILE_REQUEST;
    request->filename_size = filename_size;
    strcpy(request->filename, argv[2]);

    len = sizeof(struct FileRequest) + filename_size;
    if (sendto(s, request, len, 0, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
        perror("Client: sendto()");
        return 0;
    }

    // Receive response from server
    struct ServerMessage response;
    struct Acknowledgment ack;
    ssize_t recv_data;
    while ((recv_data = recvfrom(s, &response, sizeof(struct ServerMessage)+BUF_SIZE, 0, (struct sockaddr *)&sin, &server_addr_len)) > 0) {
        // Check response type
        if (response.type == FILE_NOT_FOUND) {
            printf("File not found on server\n");
            exit(1);
        } else if (response.type == FILE_INFO_AND_DATA) {
            printf("Received file info and data\n");
            // Receive and write file data
            uint32_t remaining_bytes = response.file_size;
            printf("Client: File size: %d\n", response.file_size);

            while (remaining_bytes > 0) {
                fwrite(response.data, 1, BUF_SIZE, fp);
                remaining_bytes -= sizeof(&response.data);
                printf("Client: Remaining bytes: %d\n", remaining_bytes);
                printf("Client: receved sequence number %d\n", response.sequence_number);
                ack.type = ACK;
                ack.sequence_no[0] = response.sequence_number;
                sendto(s, &ack, sizeof(struct Acknowledgment), 0, (struct sockaddr *)&sin, sizeof(sin));
                len = recvfrom(s, &response, recv_data, 0, (struct sockaddr *)&sin, &server_addr_len);
                printf("Client: receved Packet size %d\n", len);
            }
            printf("File received successfully\n");
            break;
        } else {
            printf("Invalid response type received\n");
            exit(1);
        }
    }

    fclose(fp);
    close(s);

    return 0;
}
