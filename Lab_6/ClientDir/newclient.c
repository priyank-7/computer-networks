

// ./client -p 5433 -f sample.txt -s 0.0.0.0
// argunment can be in any order

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

//#define SERVER_PORT 5432
#define MAX_LINE 256
#define GET "GET\0"




int main(int argc, char * argv[]) {
    FILE *fp;
    struct hostent *hp;
    struct sockaddr_in sin;
    char *host;
    char buf[MAX_LINE];
    int s;
    int len;
    char *filename;
    int SERVER_PORT;
    long filesize;
    size_t tbr;

    for (int i = 1; i < argc; i++) {
        if(strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            SERVER_PORT = atoi(argv[i + 1]);
        }
        else if (strcmp(argv[i], "-s") == 0 && i + 1 < argc) {
            host = argv[i + 1];
        }
        else if (strcmp(argv[i], "-f") == 0 && i + 1 < argc) {
            filename = argv[i + 1];
        }
    }

    /* translate host name into peer's IP address */
    hp = gethostbyname(host);
    if (!hp) {
        fprintf(stderr, "%s: unknown host: %s\n", argv[0], host);
        exit(1);
    }
    else
        printf("Client's remote host: %s\n", argv[1]);
    
    /* build address data structure */
    bzero((char *)&sin, sizeof(sin));
    sin.sin_family = AF_INET;
    bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length);
    sin.sin_port = htons(SERVER_PORT);
    
    /* active open */
    if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("simplex-talk: socket");
        exit(1);
    }
    else{
        printf("Client created socket.\n");
    }

    if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
        perror("simplex-talk: connect");
        close(s);
        exit(1);
    }
    else{
        printf("Client connected.\n");
    }



    // Send command to server
    printf("Sending command to server.\n");
    send(s, GET, sizeof(GET), 0);

    // Receive acknowledgment from server
    if (recv(s, buf, sizeof(buf), 0) <= 0 && strcmp(buf, "ACK") != 0){
        fprintf(stderr, "Error: Server did not acknowledge the command.\n");
        exit(1);
    }


    send(s, filename, strlen(filename), 0);     // Sending filename to server
    recv(s, &filesize, sizeof(filesize), 0);    // Receive acknoladgement as file size from server
    printf("file size: %ld bytes\n", filesize);
   


    FILE *file = fopen(filename, "w");
    if (!file) {
        perror("Error opening file");
        exit(1);
    }

    tbr = 0;
    while (tbr < filesize) {
        ssize_t br = recv(s, buf, sizeof(buf), 0);
        if (br <= 0) {
            fprintf(stderr, "Error while receiving file content\n");
            break;
        }
        fwrite(buf, 1, br, file);
        tbr += br;
    }
    printf("Total receves data: %ld\n", tbr);
    fclose(file);
    if (tbr != filesize) {
        fprintf(stderr, "Error: File size mismatch\n");
        exit(1);
    }
    else{
        printf("File transfer completed.\n");
    }

    close(s);
}
