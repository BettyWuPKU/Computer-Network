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
  rtp_listen(receiver_fd, 1);
  // accept the rtp connection
  rtp_accept(receiver_fd, (struct sockaddr*)&sender, &addr_len);

  char buffer[BUFFER_SIZE];

  bool * ACK = malloc(window_size * sizeof(bool));
  char * recv_window = malloc(window_size * MAXBLOCKSZ * sizeof(char));
  int * recv_len = malloc(window_size * sizeof(int));
  int expect_seq = 0;
  int fd = open(file_name, O_CREAT | O_RDWR, S_IRWXU);

  // receive packet
  while(true){
    recv_bytes = rtp_recvfrom(receiver_fd, (void *)buffer, sizeof(buffer), 0, (struct sockaddr*)&sender, &addr_len);
    if(recv_bytes == -1 || recv_bytes == -2) continue;
    buffer[recv_bytes + sizeof(rtp_header_t)] = '\0';
    rtp_header_t *rtp = (rtp_header_t *)buffer;

    //finish the connection
    if(rtp->type == RTP_END){
      // receive end message 处理丢包！！
      rtp_sendto(receiver_fd, NULL, 0, 0, (struct sockaddr*)&sender, addr_len, RTP_ACK, rtp->seq_num);
      break;
    }

    // ignore the START message
    else if(rtp->type == RTP_START) { 
      continue;
    }

    // receive packet
    else if(rtp->type == RTP_DATA) {
      int pos = expect_seq % window_size;
      // if is the expected packet, then move window forward
      if((rtp->seq_num - expect_seq < window_size) && (rtp->seq_num - expect_seq >= 0)){
        printf("sendACK: %d\n", rtp->seq_num);
        rtp_sendto(receiver_fd, NULL, 0, 0, (struct sockaddr*)&sender, addr_len, RTP_ACK, rtp->seq_num);
      }
      if(rtp->seq_num == expect_seq) {
        ACK[pos] = true;
        memcpy(recv_window + pos * MAXBLOCKSZ, buffer + sizeof(rtp_header_t), rtp->length); // copy to the window buffer
        recv_len[pos] = rtp->length;
        while(ACK[pos]){
          ACK[pos] = false;
          // printf("receive %dth msg: %s\n", expect_seq, recv_window + pos * MAXBLOCKSZ); // print the result
          write(fd, recv_window + pos * MAXBLOCKSZ, recv_len[pos]); // write the result into the file
          expect_seq++; 
          pos = expect_seq % window_size;
        }
      }
      // if not the expected one but can also be buffered and not buffered before
      else if(((rtp->seq_num - expect_seq) < window_size) && !ACK[pos] && ((rtp->seq_num - expect_seq) > 0)) {
        ACK[pos] = true;
        memcpy(recv_window + pos * MAXBLOCKSZ, buffer + sizeof(rtp_header_t), rtp->length); // copy to the window buffer
        recv_len[pos] = rtp->length;
        recv_window[pos * MAXBLOCKSZ + recv_len[pos]] = '\0';
      }
    }
  }
  close(fd);
  printf("close file\n");
  rtp_close(receiver_fd);
  free(ACK);
  free(recv_window);
  free(recv_len);
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
