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



int main(int argc, char * argv[]){
  
  FILE *fp;
  struct hostent *hp;
  struct sockaddr_in sin;
  char *host;
  char buf[BUF_SIZE];
  int s;
  int len;
  socklen_t server_addr_len;

  if ((argc==2)||(argc == 3)) {
    host = argv[1];
  }
  else {
    fprintf(stderr, "usage: client serverIP [download_filename(optional)]\n");
    exit(1);
  }

  host = argv[1];

  if(argc == 3) {
    fp = fopen(argv[2], "wb");
    if (fp == NULL) {
      fprintf(stderr, "Error opening output file\n");
      exit(1);
    }
  }

  /* translate host name into peer's IP address */
  hp = gethostbyname(host);
  if (!hp) {
    fprintf(stderr, "client: unknown host: %s\n", host);
    exit(1);
  }
  else
    printf("Host %s found!\n", argv[1]);

  /* build address data structure */
  memset((char *)&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  memcpy((char *)&sin.sin_addr, hp->h_addr, hp->h_length);
  sin.sin_port = htons(SERVER_PORT);
  

  /* create socket */
  if ((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("client: socket");
    exit(1);
  }
  
  printf("Client will get data from to %s:%d.\n", argv[1], SERVER_PORT);
  printf("To play the music, pipe the downlaod file to a player, e.g., ALSA, SOX, VLC: cat recvd_file.wav | vlc -\n"); 
  
  /* send message to server */ 
    strcpy(buf, "GET");
    len = strlen(buf) + 1;
    if (sendto(s, buf, len, 0, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
        perror("Client: sendto()");
        return 0;
    }

      int flag = 1;
      pid_t fork_result;
      if (flag == 1)
      {
        fork_result = fork();
        if (fork_result == 0)
        {
          sleep(1);
          system("open -a VLC sample_out.mp4");
        }
      }
      ssize_t recv_Data;
      int packet = 0;
      while (fork_result > 0 && (recv_Data = recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr *)&sin, &server_addr_len)) > 0) {
          fwrite(buf, 1, recv_Data, fp);
          packet++;
          recv_Data = 0;
          printf("Packet %d received\n", packet);
      }
      fclose(fp); 

  /* Add code to receive unlimited data and either display the data
     or if specified by the user, store it in the specified file. 
     Instead of recv(), use recvfrom() call for receiving data */
  recv(s, buf, sizeof(buf), 0);
  fputs(buf, stdout);
  
}