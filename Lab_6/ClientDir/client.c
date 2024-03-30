#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>


#define SERVER_PORT 5432
#define MAX_LINE 256
#define GET "GET\0"

int main(int argc, char * argv[]){
    FILE *fp;
    struct hostent *hp;
    struct sockaddr_in sin;
    char *host;
    char buf[MAX_LINE];
    int s;
    int len;
    char *filename;
    // if (argc != 5) {

    //     if(argv[1] == 's' && argv[2] != NULL){
    //         host = argv[2];
    //     }
    //     else{
    //         fprintf(stderr, "usage: %s host filename command output\n", argv[0]);
    //         exit(1);
    //     }
    //     if(argv[3] == 'f' && argv[4] != NULL){
    //         char *filename = argv[4];
    //     }
    //     else{
    //         fprintf(stderr, "usage: %s host filename command output\n", argv[0]);
    //         exit(1);
    //     }
    // }

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-s") == 0 && i + 1 < argc) {
            host = argv[i + 1];
        } else if (strcmp(argv[i], "-f") == 0 && i + 1 < argc) {
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
    else
        printf("Client created socket.\n");

    if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
        perror("simplex-talk: connect");
        close(s);
        exit(1);
    }
    else
        printf("Client connected.\n");


    // Send command to server
    printf("Sending command to server.\n");
    // fgets(buf, sizeof(buf), stdin);
    // buf[strlen(buf) - 1] = '\0'; // Remove newline character
    send(s, GET, sizeof(GET), 0);

    /* main loop: get and send lines of text */
    if(recv(s, buf, sizeof(buf), 0) > 0){
        send(s, filename, sizeof(buf), 0);
    }   
    else{
        fprintf(stderr, "Error: Server did not acknowledge the command.\n");
        exit(1);
    }
    FILE *file = fopen(filename, "w");
    while (recv(s, buf, sizeof(buf), 0) > 0) {
        // printf("%s", buf); // Print received content
        if(strcmp(buf, "404 File Not Found\n") == 0){
            fprintf(stderr, "Error: File not found.\n");
            break;
        }
        else{
            fwrite(buf, 1, sizeof(buf), file);
        }
    }
    close(s);
}
