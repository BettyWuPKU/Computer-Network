#ifndef RTP_H
#define RTP_H

#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <fcntl.h>
#include <time.h>

#define RTP_START 0
#define RTP_END   1
#define RTP_DATA  2
#define RTP_ACK   3
#define EXCEED_TIME 500
#define BUFFER_SIZE 2048
#define RECV_BUFFER_SIZE 32768  // 32KB
#define MAX_RAND 10000000
#define MAX(x, y) x>y? x:y
#define MIN(x, y) x<y? x:y

typedef struct __attribute__ ((__packed__)) RTP_header {
    uint8_t type;       // 0: START; 1: END; 2: DATA; 3: ACK
    uint16_t length;    // Length of data; 0 for ACK, START and END packets
    uint32_t seq_num;
    uint32_t checksum;  // 32-bit CRC
} rtp_header_t;

#define MAXBLOCKSZ (1472 - sizeof(rtp_header_t))

typedef struct RTP_control_block {
    uint32_t window_size;
    // TODO: you can add your RTP-related fields here
} rcb_t;

static rcb_t* rcb = NULL;

// different from the POSIX
int rtp_socket(uint32_t window_size);

int rtp_listen(int sockfd, int backlog);

int rtp_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);

int rtp_bind(int sockfd, struct sockaddr *addr, socklen_t addrlen);

int rtp_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen, int window_size);

int rtp_close(int sockfd);

int rtp_sendto(int sockfd, const void *msg, int len, int flags, const struct sockaddr *to, socklen_t tolen, uint8_t, uint32_t);

int rtp_recvfrom(int sockfd, void *buf, int len, int flags, struct sockaddr *from, socklen_t *fromlen);

int rtp_end(int sockfd, const void *msg, int len, int flags, const struct sockaddr *to, socklen_t tolen);
#endif //RTP_H
