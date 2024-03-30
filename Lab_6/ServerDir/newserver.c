#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

// #define SERVER_PORT 5432
#define MAX_PENDING 5
#define MAX_LINE 256
#define ACK "ACK\0"
#define ERROR_404 "404 File Not Found\n"



//./server -p 5433



int main(int argc, char * argv[]) {
    struct sockaddr_in sin;
    char buf[MAX_LINE];
    socklen_t len;
    int s, new_s;
    char str[INET_ADDRSTRLEN];
    char *fatch_filename;
    int SERVER_PORT;

    for (int i = 1; i < argc; i++) {
        if(strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            SERVER_PORT = atoi(argv[i + 1]);
        }
    }

    /* build address data structure */
    bzero((char *)&sin, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(SERVER_PORT);
    
    /* setup passive open */
    if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("simplex-talk: socket");
        exit(1);
    }
    
    inet_ntop(AF_INET, &(sin.sin_addr), str, INET_ADDRSTRLEN);
    printf("Server is using address %s and port %d.\n", str, SERVER_PORT);

    if ((bind(s, (struct sockaddr *)&sin, sizeof(sin))) < 0) {
        perror("simplex-talk: bind");
        exit(1);
    }
    else
        printf("Server bind done.\n");

    listen(s, MAX_PENDING);
    
    /* wait for connection, then receive and print text */
    while(1) {
        printf("Server Listening.\n");
        if ((new_s = accept(s, (struct sockaddr *)&sin, &len)) < 0) {
            perror("simplex-talk: accept");
            exit(1);
        }
        printf("Server Listening.\n");
        // Receive command from client
        recv(new_s, buf, sizeof(buf), 0);
        // Check if the command is "GET"
        if (strcmp(buf, "GET") == 0) {
            printf("GET Command received: %s\n", buf);
            send(new_s, ACK, sizeof(ACK), 0);   // sending acknolodgment
            sleep(1);
            recv(new_s, buf, sizeof(buf), 0);   // receving file name
            fatch_filename = buf;

            FILE *file = fopen(fatch_filename, "r");
            if (file == NULL) {
                printf("404 File Not Found\n");
                send(new_s, ERROR_404, strlen(ERROR_404), 0);
            }
            else {
                // calculating file size
                fseek(file, 0, SEEK_END);
                long filesize = ftell(file);
                fseek(file, 0, SEEK_SET);
                send(new_s, &filesize, sizeof(filesize), 0);
                size_t rff;
                while ((rff = fread(buf, 1, sizeof(buf), file)) > 0) {
                    send(new_s, buf, rff, 0);
                }
                fclose(file);
            }
            for (int i = 0; i < sizeof(buf); i++) {
                    buf[i] = '\0';
            }
        }
        else {
            printf("Invalid command.\n");
        }
        close(new_s);
    }
}
