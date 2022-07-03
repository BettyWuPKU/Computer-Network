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
  read_timeout.tv_sec = 5;
  read_timeout.tv_usec = 500;
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
  char buffer[BUFFER_SIZE];
  struct sockaddr_in sender;
  socklen_t addr_len = sizeof(struct sockaddr_in);

  printf("test1\n");
  int ret1 = rtp_sendto(sock, NULL, 0, 0, (const struct sockaddr *) &receiver_addr, sizeof(struct sockaddr), RTP_START, 0);
  printf("ret1:%d\n", ret1);
  int ret2 = recvfrom(sock, buffer, 2048, 0, (struct sockaddr *)&sender, &addr_len);
  printf("ret2:%d\n", ret2);

  // close rtp socket
  rtp_close(sock);

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