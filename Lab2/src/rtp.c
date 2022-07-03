#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>

#include "rtp.h"
#include "util.h"


void rcb_init(uint32_t window_size) {
    if (rcb == NULL) {
        rcb = (rcb_t *) calloc(1, sizeof(rcb_t));
    } else {
        perror("The current version of the rtp protocol only supports a single connection");
        exit(EXIT_FAILURE);
    }
    rcb->window_size = window_size;
    // TODO: you can initialize your RTP-related fields here
}

/*********************** Note ************************/
/* RTP in Assignment 2 only supports single connection.
/* Therefore, we can initialize the related fields of RTP when creating the socket.
/* rcb is a global varialble, you can directly use it in your implementatyion.
/*****************************************************/
int rtp_socket(uint32_t window_size) {
    rcb_init(window_size); 
    // create UDP socket
    return socket(AF_INET, SOCK_DGRAM, 0);  
}


int rtp_bind(int sockfd, struct sockaddr *addr, socklen_t addrlen) {
    return bind(sockfd, addr, addrlen);
}


int rtp_listen(int sockfd, int backlog) {
    // TODO: listen for the START message from sender and send back ACK
    // In standard POSIX API, backlog is the number of connections allowed on the incoming queue.
    // For RTP, backlog is always 1
    int start_seq_num;
    int recv_bytes;
    struct sockaddr_in sender;
    socklen_t addr_len = sizeof(struct sockaddr_in);
    char buffer[BUFFER_SIZE];
    // receive start message & send back ACK & missing END ACK
    while(true){
        recv_bytes = rtp_recvfrom(sockfd, (void *)buffer, sizeof(buffer), 0, (struct sockaddr*)&sender, &addr_len);
        if(recv_bytes == -1) continue;
        // checksum error
        else if(recv_bytes == -2) return -1;
        buffer[recv_bytes + sizeof(rtp_header_t)] = '\0';
        rtp_header_t *rtp = (rtp_header_t *)buffer;
        if(rtp->type == RTP_START){
            // send ACK to accept the connection
            printf("receive STARTmsg\n");
            start_seq_num = rtp->seq_num;
            rtp_sendto(sockfd, NULL, 0, 0, (struct sockaddr*)&sender, addr_len, RTP_ACK, rtp->seq_num);
            break;
        }
        else if(rtp->type == RTP_END){
            // send delayed END to close last connection
            printf("receive delayed END\n");
            rtp_sendto(sockfd, NULL, 0, 0, (struct sockaddr*)&sender, addr_len, RTP_ACK, rtp->seq_num);
            break;
        }
    }
    return 1;
}


int rtp_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
    // Since RTP in Assignment 2 only supports one connection,
    // there is no need to implement accpet function.
    // You don’t need to make any changes to this function.
    return 1;
}

// used by the sender to send the START message and wait for ACK for START
// return 1 if succeed, else return 0 and later send END
int rtp_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen, int window_size) {
    // TODO: send START message and wait for its ACK
    // send START message
    int seq_num = (rand() * MAX_RAND) + 2 * window_size;
    rtp_sendto(sockfd, NULL, 0, 0, addr, addrlen, RTP_START, seq_num);

    // wait for ACK
    int recv_bytes;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in sender;
    clock_t send_time = clock();
    
    socklen_t addr_len = sizeof(struct sockaddr_in);
    if ((recv_bytes = rtp_recvfrom(sockfd, (void *)buffer, sizeof(buffer), 0, (struct sockaddr*)&sender, &addr_len)) < 0) {
        return 0;
    }
    rtp_header_t *rtp = (rtp_header_t *)buffer;
    if(rtp->type == RTP_ACK && rtp->seq_num == seq_num){
        printf("receive START_ACK\n");
        return 1;
    }
    else 
        printf("wrong message.\n");
    // // if exceed 500ms then resend start request
    // if(clock() - send_time >= EXCEED_TIME) {
    //     return -1;
    // }

    return 0;
}

int rtp_close(int sockfd) {
    return close(sockfd);
}

// return 0 if the END ACK message is missing, return 1 if receive the END ACK successfully
int rtp_end(int sockfd, const void *msg, int len, int flags, const struct sockaddr *to, socklen_t tolen) {
    int recv_bytes;
    char recv_buf[BUFFER_SIZE];
    struct sockaddr_in sender;
    socklen_t addr_len = sizeof(struct sockaddr_in);
    
    int seq_num = (rand() * MAX_RAND) + MAX_RAND;
    // send END msg
    rtp_sendto(sockfd, NULL, 0, 0, to, tolen, RTP_END, seq_num);
    if ((recv_bytes = rtp_recvfrom(sockfd, (void *)recv_buf, sizeof(recv_buf), 0, (struct sockaddr*)&sender, &addr_len)) < 0) {
        printf("END ACK missing, the connection is closed.\n");
        return 0;
    }
    rtp_header_t *rtp = (rtp_header_t *)recv_buf;
    if(rtp->type == RTP_ACK && rtp->seq_num == seq_num)
        return 1;
    // wrong ACK msg
    printf("END ACK missing, the connection is closed.\n");
    return 0;
}


int rtp_sendto(int sockfd, const void *msg, int len, int flags, const struct sockaddr *to, socklen_t tolen, uint8_t type, uint32_t seq_num) {
    // TODO: send message

    // Send the first data message sample
    char buffer[BUFFER_SIZE];
    rtp_header_t* rtp = (rtp_header_t*)buffer;
    rtp->length = len;
    rtp->checksum = 0;
    rtp->seq_num = seq_num;
    rtp->type = type;
    if(type == RTP_DATA)
        memcpy((void *)buffer+sizeof(rtp_header_t), msg, len);
    rtp->checksum = compute_checksum((void *)buffer, sizeof(rtp_header_t) + len);
    // printf("send type:%d\n", rtp->type);
    // printf("send checksum:%d\n", rtp->checksum);

    int sent_bytes = sendto(sockfd, (void*)buffer, sizeof(rtp_header_t) + len, flags, to, tolen);
    if (sent_bytes != (sizeof(struct RTP_header) + len)) {
        perror("send error");
        exit(EXIT_FAILURE);
    }
    return 1;

}

// return -1 if time exceed, else return the length of msg(not including the header); return -2 if checksum error
int rtp_recvfrom(int sockfd, void *buf, int len, int flags,  struct sockaddr *from, socklen_t *fromlen) {
    // TODO: recv message

    char buffer[2048];
    int recv_bytes = recvfrom(sockfd, buffer, 2048, flags, from, fromlen);
    if (recv_bytes < 0) {
        return -1;
    }
    buffer[recv_bytes] = '\0';
    // extract header
    rtp_header_t *rtp = (rtp_header_t *)buffer;
    
    // verify checksum
    uint32_t pkt_checksum = rtp->checksum;
    rtp->checksum = 0;
    uint32_t computed_checksum = compute_checksum(buffer, recv_bytes);
    if (pkt_checksum != computed_checksum) {
        perror("checksums not match");
        return -2;
    }  
    memcpy(buf, buffer, sizeof(rtp_header_t) + rtp->length);
    return rtp->length;
}