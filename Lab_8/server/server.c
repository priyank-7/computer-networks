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
#include <pthread.h>

#define SERVER_PORT 5432
#define BUF_SIZE 4096
#define MAX_FILENAME_SIZE 256
#define MAX_PENDING  5   // Maximum outstanding packets
#define TIMEOUT 2  // Timeout interval in seconds 

// Message Structures
struct File_request{
    uint8_t type;
    uint8_t filename_size;
    char filename[MAX_FILENAME_SIZE];
};

struct ACK{
    uint8_t type;
    uint8_t num_sequences;  
    uint16_t sequence_no[MAX_PENDING];
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
    uint16_t block_size;
    char data[BUF_SIZE]; // Adjust based on header size
};

struct File_not_found{
    uint8_t type;
    uint8_t filename_size;
    char filename[MAX_FILENAME_SIZE];
};

int isTimeOut = 0;
void *packet_timer(){
    struct timespec req;
    req.tv_sec = 0; // Initialize to 0 seconds
    req.tv_nsec = 10000; // 10 milliseconds
    nanosleep(&req, NULL);
    isTimeOut = 1;
    return NULL;
}

void send_data(int sockfd, struct File_request file_request, int len, struct sockaddr *dest_addr, socklen_t addrlen, FILE *fp) {

    struct File_info_and_data file_info_and_data;
    memset(&file_info_and_data, 0, sizeof(file_info_and_data));
    printf("Size of file_info_and_data is %ld\n", sizeof(file_info_and_data));

    struct Data data;
    memset(&data, 0, sizeof(data));
    printf("Size of data is %ld\n", sizeof(data));
    // Getting file size and decide the number of sequences
    fseek(fp, 0, SEEK_END);
    uint32_t file_size = ftell(fp);
    rewind(fp);
    uint16_t num_sequences = (file_size / (BUF_SIZE - sizeof(uint16_t) - sizeof(uint32_t))) + 1;
    char buf[BUF_SIZE];
    memset(buf, 0, BUF_SIZE);
    printf("Size of buf is %ld\n", sizeof(buf));
    struct ACK ack;
    memset(&ack, 0, sizeof(ack));
    printf("Size of ack is %ld\n", sizeof(ack));

    uint8_t seq_packet = 0;
    // malloc(sizeof(int) * window_size);
    
    file_info_and_data.type = 2;
    file_info_and_data.sequence_number = 0;
    file_info_and_data.filename_size = file_request.filename_size;
    strcpy(file_info_and_data.filename, file_request.filename);
    file_info_and_data.file_size = file_size;
    file_info_and_data.block_size = BUF_SIZE;
    int data_size = fread(buf, 1, BUF_SIZE, fp);
    memcpy(file_info_and_data.data, buf, BUF_SIZE);
    printf("Size of file_info_and_data is %ld\n", sizeof(file_info_and_data));
    printf("Size of file_info_and_data.data is %ld\n", sizeof(file_info_and_data.data));
    printf("Client: receved File size %ul", file_info_and_data.file_size);
 
    pthread_t timer_thread;

    len = sendto(sockfd, &file_info_and_data, sizeof(file_info_and_data), 0, dest_addr, addrlen);
    seq_packet++;
    while((data_size = recvfrom(sockfd, &ack, sizeof(ack), 0, dest_addr, &addrlen)) > 0){
            printf("Size of ack is %ld\n", sizeof(ack));
            //uint8_t temp_seq_number = ack.num_sequences;
            // ... (Send packets in the window)
            if(ack.type == 1){
                // ... (Update sliding)
                void pthread_exit(void * retval);
                memset(&data, 0, sizeof(data));
                memset(&buf, 0, BUF_SIZE);
                memset(&ack, 0, sizeof(ack));
                data_size = fread(buf, 1, BUF_SIZE, fp); 
                data.type = 3;
                data.sequence_number = seq_packet;
                data.block_size = data_size;
                memcpy(data.data, buf, data_size);
                printf("Buffer Data is %s\n", data.data);
                isTimeOut = 0;
                if(data_size  <= 0){
                    break;
                }
                seq_packet++;
                len = sendto(sockfd, &data, sizeof(data), 0, dest_addr, addrlen);
                pthread_create(&timer_thread, NULL, packet_timer, NULL);
            }
            else{
                // ... (Resend packets)
                void pthread_exit(void * retval);
                len = sendto(sockfd, &data, sizeof(data), 0, dest_addr, addrlen);
                pthread_create(&timer_thread, NULL, packet_timer, NULL);
                isTimeOut = 0;
            }
            printf("Data sent successfully\n");
    }
}

int main(int argc, char *argv[]) {

    // ... (Socket setup, address binding - similar to your template)
  struct sockaddr_in sin;
  struct sockaddr_storage client_addr;
  char clientIP[INET_ADDRSTRLEN]; /* For IPv4 addresses */
  socklen_t client_addr_len;
  int len;
  char buf[BUF_SIZE];
  int s;
  char *host;
  struct hostent *hp;
  struct File_request file_request;
  struct File_not_found file_not_found;
  struct ACK ack;
  struct File_info_and_data file_info_and_data;


  /* Declarations for file(s) to be sent 
     ...
  */
    FILE *fp;
    char *filename;
  
  /* For inserting delays, use nanosleep()
    struct timespec ... */ 
    struct timespec req;
    req.tv_sec = 0; // Initialize to 0 seconds
    req.tv_nsec = 10000; // 500 milliseconds
 

  /* To get filename from commandline */
  /* if (argc==...) {} */
    


    
   
  /* Create a socket */
  if ((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("server: socket");
    exit(1);
  }
 
 
  /* build address data structure and bind to all local addresses*/
  memset((char *)&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
 
  /* If socket IP address specified, bind to it. */
  if(argc == 2) {
    host = argv[1];
    hp = gethostbyname(host);
    if (!hp) {
      fprintf(stderr, "server: unknown host %s\n", host);
      exit(1);
    }
    memcpy((char *)&sin.sin_addr, hp->h_addr, hp->h_length);
  }
  /* Else bind to 0.0.0.0 */
  else
    sin.sin_addr.s_addr = INADDR_ANY;
  
  sin.sin_port = htons(SERVER_PORT);
  
  if ((bind(s, (struct sockaddr *)&sin, sizeof(sin))) < 0) {
    perror("server: bind");
    exit(1);
  }
  else{
    /* Add code to parse IPv6 addresses */
    inet_ntop(AF_INET, &(sin.sin_addr), clientIP, INET_ADDRSTRLEN);
    printf("Server is listening at address %s : %d \n", clientIP, SERVER_PORT);
  }
  
  printf("Client needs to send \"GET\" to receive the file %s\n", argv[1]);
    
  
    client_addr_len = sizeof(client_addr); 
    
    while (1) {
        memset(buf, 0, BUF_SIZE);
        len = recvfrom(s, &file_request, BUF_SIZE, 0, (struct sockaddr *) &client_addr, &client_addr_len);

        // ... (logging)

        // Message Handling (add more cases as needed)
        if (file_request.type == 0) {
            // ... (Open the file)
            fp = fopen(file_request.filename, "rb");

            if (fp == NULL) { // File found
                // ... (Get filesize) 
                //file_not_found.type = 4;
                printf("File name is %s\n", file_request.filename);
                file_info_and_data.type = 4;
                file_info_and_data.filename_size = file_request.filename_size;
                strncpy(file_info_and_data.filename, file_request.filename, file_request.filename_size);
                sendto(s, &file_info_and_data, sizeof(&file_info_and_data), 0, (struct sockaddr *)&client_addr, client_addr_len);
                // free(&file_not_found);
                // Start main data transmission loop ... (see below)
            } else { // File not found
                // ... (Send file not found message)
                send_data(s, file_request, len, (struct sockaddr *)&client_addr, client_addr_len, fp);
            }
        }
        else{
            printf("Invalid request type\n");
        }
        printf("File sent successfully\n");
    }
}
