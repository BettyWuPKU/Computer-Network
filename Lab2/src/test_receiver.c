#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>

#include "util.h"
#include "rtp.h"

int receiver(char *receiver_port, int window_size, char* file_name) {
// START_NEW_CONNECTION:
  // create rtp socket file descriptor
  int receiver_fd = rtp_socket(window_size);
  if (receiver_fd == 0) {
    perror("create rtp socket failed");
    exit(EXIT_FAILURE);
  }

  // create socket address
  // forcefully attach socket to the port
  struct sockaddr_in address;
  memset(&address, 0, sizeof(struct sockaddr_in));
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(atoi(receiver_port));

  // bind rtp socket to address
  if (rtp_bind(receiver_fd, (struct sockaddr *)&address, sizeof(struct sockaddr))<0) {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }

  int recv_bytes;
  struct sockaddr_in sender;
  socklen_t addr_len = sizeof(struct sockaddr_in);

  // listen to incoming rtp connection
  // rtp_listen(receiver_fd, 1);
  // accept the rtp connection
  rtp_accept(receiver_fd, (struct sockaddr*)&sender, &addr_len);

  int start_seq_num;
  char buffer[BUFFER_SIZE];
  // receive start message & send back ACK & missing END ACK
  while(true){
    if ((recv_bytes = rtp_recvfrom(receiver_fd, (void *)buffer, sizeof(buffer), 0, (struct sockaddr*)&sender, &addr_len)) < 0) {
      perror("receive error");
    }
    buffer[recv_bytes + sizeof(rtp_header_t)] = '\0';
    rtp_header_t *rtp = (rtp_header_t *)buffer;
    printf("recv type:%d\n", rtp->type);
    printf("recv msg:");
    for(int i = 0; i < recv_bytes + sizeof(rtp_header_t); i++) printf("%c", buffer[i]);
    printf("\n");
    if(rtp->type == RTP_START){
      // send ACK to accept the connection
      printf("receive START msg\n");
      start_seq_num = rtp->seq_num;
      rtp_sendto(receiver_fd, NULL, 0, 0, (struct sockaddr*)&sender, addr_len, RTP_ACK, rtp->seq_num);
      break;
    }
    else if(rtp->type == RTP_END){
      // send delayed END to close last connection
      printf("receive delayed END\n");
      rtp_sendto(receiver_fd, NULL, 0, 0, (struct sockaddr*)&sender, addr_len, RTP_ACK, rtp->seq_num);
      break;
    }
  }

  
  rtp_close(receiver_fd);   

  return 0;
}

/*
 * main():
 * Parse command-line arguments and call receiver function
*/
int main(int argc, char **argv) {
    char *receiver_port;
    int window_size;
    char *file_name;

    if (argc != 4) {
        fprintf(stderr, "Usage: ./receiver [Receiver Port] [Window Size] [File Name]\n");
        exit(EXIT_FAILURE);
    }

    receiver_port = argv[1];
    window_size = atoi(argv[2]);
    file_name = argv[3];
    return receiver(receiver_port, window_size, file_name);
}
