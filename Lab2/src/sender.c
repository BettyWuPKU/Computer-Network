#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "util.h"
#include "rtp.h"


int sender(char *receiver_ip, char* receiver_port, int window_size, char* message){
  struct timeval read_timeout;
  read_timeout.tv_sec = 0;
  read_timeout.tv_usec = EXCEED_TIME;
  // create socket
  int sock = 0;
  if ((sock = rtp_socket(window_size)) < 0) {
    perror("socket failed");
    exit(EXIT_FAILURE);
  }
  // revise the recvfrom to non blocking function
  setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &read_timeout, sizeof(read_timeout));

  // create receiver address
  struct sockaddr_in receiver_addr;
  memset(&receiver_addr, 0, sizeof(receiver_addr));
  receiver_addr.sin_family = AF_INET;
  receiver_addr.sin_port = htons(atoi(receiver_port));

  // convert IPv4 or IPv6 addresses from text to binary form
  if(inet_pton(AF_INET, receiver_ip, &receiver_addr.sin_addr)<=0) {
    perror("address failed");
    exit(EXIT_FAILURE);
  }

  // connect to server, send START message
  int start_succ = rtp_connect(sock, (struct sockaddr *)&receiver_addr, sizeof(struct sockaddr), window_size);
  if(start_succ == 0) {
    printf("START ACK missing.\n");
    int ans = rtp_end(sock, NULL, 0, 0, (struct sockaddr*)&receiver_addr, sizeof(struct sockaddr));
    if(ans == -1){
      rtp_close(sock);
      return -1;
    }
    printf("receive END_ACK\n");
    rtp_close(sock);
    return -1;
  }

  // send data
  // TODO: if message is filename, open the file and send its content
  int start = 0, end = 0; // the first one no ACK and the next one not sent
  // int seq_range = 2 * window_size; // 防止出现ACK延迟造成上一个已经确认过的包在下一个window中再次被确认造成错误！！不太确定这个处理对不对
  char * send_window = malloc(MAXBLOCKSZ * window_size * sizeof(char));
  int * block_size = malloc(window_size * sizeof(int));
  int recv_bytes;
  struct sockaddr_in sender;
  socklen_t addr_len = sizeof(struct sockaddr_in);
  char recv_buf[BUFFER_SIZE];
  int fd, offset = 0, len;
  bool finish_read = false;
  clock_t send_time;

  // if it is a filename
  if((fd = open(message, O_RDWR)) != -1){
    char buf[MAXBLOCKSZ];
    while(true) {
      // available to send more packets
      if(end - start < window_size && !finish_read) {
        len = read(fd, buf, sizeof(buf));
        // still have file content to be sent
        if(len != 0) {
          int pos = offset % window_size;
          rtp_sendto(sock, (void *)buf, len, 0, (struct sockaddr*)&receiver_addr, sizeof(struct sockaddr), RTP_DATA, offset);
          memcpy(send_window + pos * MAXBLOCKSZ, buf, len); // buffer the sent message
          block_size[pos] = len;
          offset++;
          end++;
          send_time = clock();
        }
        else {finish_read = true;}
      }
      if(start == end && finish_read) {
        close(fd);
        break;
      }
      // unavailable to send more, wait for more ACK
      recv_bytes = rtp_recvfrom(sock, (void *)recv_buf, sizeof(recv_buf), 0, (struct sockaddr*)&sender, &addr_len);
      // wrong checksum
      if(recv_bytes == -2) {
        close(fd);
        break;
      }
      // time exceed-resend msg
      if(recv_bytes == -1 || clock() - send_time >= EXCEED_TIME) {
        printf("Time exceed, resend\n");
        for(int i = start; i < end; i++){
          printf("resend:%d\n", i);
          rtp_sendto(sock, (void *)send_window + MAXBLOCKSZ * (i % window_size), block_size[i % window_size], 0, \
                          (struct sockaddr*)&receiver_addr, sizeof(struct sockaddr), RTP_DATA, i);
        }
        send_time = clock();
        continue;
      }
      recv_buf[recv_bytes + sizeof(rtp_header_t)] = '\0';
      rtp_header_t *rtp = (rtp_header_t *)recv_buf;
      if(rtp->type == RTP_ACK){
        // receive ACK message
        if(start != rtp->seq_num) send_time = clock();
        start = rtp->seq_num;
      }
    }
  }
  // if not filename
  else {
    len = strlen(message);
    while(true){
      if(end - start < window_size && !finish_read){
        int temp = MIN(len, MAXBLOCKSZ);
        if(temp != 0) {
          int pos = offset % window_size;
          rtp_sendto(sock, (void *)message + offset * MAXBLOCKSZ, temp, 0, \
                      (struct sockaddr*)&receiver_addr, sizeof(struct sockaddr), RTP_DATA, offset);
          block_size[pos] = temp;
          len -= temp;
          offset++;
          end++;
          send_time = clock();
        }
        else{finish_read = true;}
      }
      if(start == end && finish_read) break;

      // unavailable to send more, wait for more ACK
      recv_bytes = rtp_recvfrom(sock, (void *)recv_buf, sizeof(recv_buf), 0, (struct sockaddr*)&sender, &addr_len);
      if(recv_bytes == -2) break;
      // time exceed-resend msg
      if(recv_bytes == -1 || clock() - send_time >= EXCEED_TIME) {
        printf("Time exceed, resend\n");
        for(int i = start; i < end; i++){
          printf("resend:%d\n", i);
          rtp_sendto(sock, (void *)send_window + MAXBLOCKSZ * (i % window_size), \
                    block_size[i % window_size], 0, (struct sockaddr*)&receiver_addr, \
                    sizeof(struct sockaddr), RTP_DATA, i);
        }
        send_time = clock();
        continue;
      }
      recv_buf[recv_bytes + sizeof(rtp_header_t)] = '\0';
      rtp_header_t *rtp = (rtp_header_t *)recv_buf;
      if(rtp->type == RTP_ACK){
        // receive ACK message
        if(start != rtp->seq_num) send_time = clock();
        start = rtp->seq_num;
      }
    }
    
  }

  if(start == end && finish_read)
    printf("finish sending message\n");

  // send END message
  int ans = rtp_end(sock, NULL, 0, 0, (struct sockaddr*)&receiver_addr, sizeof(struct sockaddr));
  if(ans == -1){
    rtp_close(sock);
    free(send_window);
    free(block_size);
    return -1;
  }
  printf("receive END_ACK\n");
  // close rtp socket
  rtp_close(sock);
  free(send_window);
  free(block_size);

  return 0;
}



/*
 * main()
 * Parse command-line arguments and call sender function
*/
int main(int argc, char **argv) {
  char *receiver_ip;
  char *receiver_port;
  int window_size;
  char *message;

  if (argc != 5) {
    fprintf(stderr, "Usage: ./sender [Receiver IP] [Receiver Port] [Window Size] [message]");
    exit(EXIT_FAILURE);
  }

  receiver_ip = argv[1];
  receiver_port = argv[2];
  window_size = atoi(argv[3]);
  message = argv[4];
  return sender(receiver_ip, receiver_port, window_size, message);
}