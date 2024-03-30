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

#define SERVER_PORT 5432
#define BUF_SIZE 4096
#define MAX_FILENAME_SIZE 256
#define First_Recv_Size 




struct File_request{
    uint8_t type;
    uint8_t filename_size;
    char filename[MAX_FILENAME_SIZE];
};

struct ACK{
    uint8_t type;
    uint8_t num_sequences;  
};

struct File_info_and_data{
    uint8_t type;
    uint16_t sequence_number;
    uint8_t filename_size;
    char filename[MAX_FILENAME_SIZE];
    uint32_t file_size;
    uint16_t block_size;
    char data[BUF_SIZE]; // Adjust based on headers 
};

struct Data{
    uint8_t type;
    uint16_t sequence_number;
    char data[BUF_SIZE]; // Adjust based on header size
};

struct File_not_found{
    uint8_t type;
    uint8_t filename_size;
    char filename[MAX_FILENAME_SIZE];
};





int main(int argc, char * argv[]){
  
    FILE *fp;
    struct hostent *hp;
    struct sockaddr_in sin;
    char *host;
    char buf[BUF_SIZE];
    int s;
    int len;
    socklen_t server_addr_len;
    char *filename;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-s") == 0 && i + 1 < argc) {
            host = argv[i + 1];
        } else if (strcmp(argv[i], "-f") == 0 && i + 1 < argc) {
            filename = argv[i + 1];
        }
    }


    fp = fopen(filename, "wb");
    if (fp == NULL) {
      fprintf(stderr, "Error opening output file\n");
      exit(1);
    }


  /* translate host name into peer's IP address */
    hp = gethostbyname(host);
    if (!hp) {
        fprintf(stderr, "client: unknown host: %s\n", host);
        exit(1);
    }
    else
        printf("Client's remote host:\n");

  /* build address data structure */

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    memcpy((char *)&sin.sin_addr, hp->h_addr, hp->h_length);
    sin.sin_port = htons(SERVER_PORT);
  

    /* create socket */
    if ((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("client: socket");
        exit(1);
    }


    struct File_request file_request;
    memset(&file_request, 0, sizeof(file_request));
    struct File_info_and_data file_info_and_data;
    memset(&file_info_and_data, 0, sizeof(file_info_and_data));
    struct Data data;
    memset(&data, 0, sizeof(data));
    struct File_not_found file_not_found;
    memset(&file_not_found, 0, sizeof(file_not_found));
    struct ACK ack;
    memset(&ack, 0, sizeof(ack));

    file_request.type = 0;
    file_request.filename_size = strlen(filename);
    strcpy(file_request.filename, filename);
    sendto(s, &file_request, sizeof(file_request), 0, (struct sockaddr *)&sin, sizeof(sin));
    int recv_Data = 0;


    // check if the response is filenot found or file info
    recvfrom(s, &file_info_and_data, 4368, 0, (struct sockaddr *)&sin, &server_addr_len);
    recv_Data = sizeof(file_info_and_data.data);
    printf("Size of receved data %lu\n", sizeof(file_info_and_data));
    printf("Client: receved File size %ul", file_info_and_data.file_size);



    if (file_info_and_data.type == 4) {
        printf("File not found\n");
        return 0;
    }


    strcpy(buf, file_info_and_data.data);

    
    while (recv_Data > 0) {
        fwrite(buf, 1, recv_Data, fp);
        memset(buf, 0, sizeof(buf));
        recv_Data = 0;
        printf("Packet %d received\n", data.sequence_number);
        ack.type = 1;
        ack.num_sequences = data.sequence_number;
        sendto(s, &ack, sizeof(ack), 0, (struct sockaddr *)&sin, sizeof(sin));
        recvfrom(s, &data, 4102, 0, (struct sockaddr *)&sin, &server_addr_len);
        printf("Size of receved data %lu\n", sizeof(data));
        strncpy(buf, data.data, sizeof(data.data));
        printf("Data data %s\n", data.data);
        printf("Buffer data %s\n", buf);
        recv_Data = sizeof(buf);
        memset(&data,0, sizeof(data));
        memset(&ack, 0, sizeof(ack));
    }
  /* send message to server */ 
    // strcpy(buf, "GET");
    // len = strlen(buf) + 1;
    // if (sendto(s, buf, len, 0, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
    //     perror("Client: sendto()");
    //     return 0;
    // }

    //   int flag = 1;
    //   pid_t fork_result;
    //   if (flag == 1)
    //   {
    //     fork_result = fork();
    //     if (fork_result == 0)
    //     {
    //       sleep(1);
    //       system("open -a VLC sample_out.mp4");
    //     }
    //   }
    //   ssize_t recv_Data;
    //   int packet = 0;
    //   while (fork_result > 0 && (recv_Data = recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr *)&sin, &server_addr_len)) > 0) {
    //       fwrite(buf, 1, recv_Data, fp);
    //       packet++;
    //       recv_Data = 0;
    //       printf("Packet %d received\n", packet);
    //   }
    fclose(fp);

  /* Add code to receive unlimited data and either display the data
     or if specified by the user, store it in the specified file. 
     Instead of recv(), use recvfrom() call for receiving data */
  // recv(s, buf, sizeof(buf), 0);
    //fputs(buf, stdout);
  
}