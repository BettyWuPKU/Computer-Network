#include "myftp.h"

char protocol[6] = {0xe3, 'm', 'y', 'f', 't', 'p'};

void set_message(struct message_s *info, char protocol[6], char type, char status, int length){
    memcpy(info->protocol, protocol, 6);
    info->type = type;
    info->status = status;
    info->length = length;
}

int set_connection(int sockfd){
    char buf[MAXLINE];
    struct message_s info_send, info_recv;
    printf("Client connection %d accepted.\n", sockfd);

    set_message(&info_send, protocol, (char)0xA2, 1, 12);
    write(sockfd, &info_send, sizeof(info_send));
    return UNAUTH;
}

int auth(int sockfd, int len){
    char buf[MAXLINE], buf2[MAXLINE];
    int payload_len;
    struct message_s info_send, info_recv;
 
    // read(sockfd, (char *)&info_recv, sizeof(info_recv));
    payload_len = len - 13;
    read(sockfd, buf, payload_len);
    // printf("%s", buf);
    FILE *fd = fopen("access.txt", "r");
    while(fgets(buf2, MAXLINE, fd) != NULL){
        // printf("%s", buf2);
        if(memcmp(buf, buf2, strlen(buf2)) == 0){
            printf("Authentication passed.\n");
            set_message(&info_send, protocol, (char)0xA4, 1, 12);
            write(sockfd, (char *)&info_send, sizeof(info_send));
            return AUTHED;
        }
    }
    // auth fail
    printf("reject the auth.\n");
    set_message(&info_send, protocol, (char)0xA4, 0, 12);
    write(sockfd, (char *)&info_send, sizeof(info_send));
    return UNCONNECTED;
}

void handler(void *connfd_p){
    int connfd = (int)(*(int *)connfd_p);
    free(connfd_p);
    struct message_s info_send, info_recv;
    char buf[MAXLINE];
    int n, state = UNCONNECTED;
AGAIN:
    while((n = read(connfd, (char *)&info_recv, sizeof(info_recv))) > 0){
        // printf("%02x %d\n", info_recv.type, info_recv.status);
        // connect
        if(info_recv.type == (char)0xA1){
            state = set_connection(connfd);
        }
        // auth
        else if(info_recv.type == (char)0xA3 && state == UNAUTH){
            state = auth(connfd, info_recv.length);
        }
        else if(state == AUTHED){
            int payload_len = 0;
            // ls 
            if(info_recv.type == (char)0xA5){
                memset(buf, 0, sizeof(buf));
                DIR* dirp = opendir("./filedir");
                if(dirp == NULL){ 
                    printf("Fail to open the dir.\n"); 
                    set_message(&info_send, protocol, (char)0xA6, unused, 13 + payload_len);
                    write(connfd, (char *)&info_send, sizeof(info_send));
                } // send empty
                else{
                    struct dirent *dp1 = malloc(sizeof(struct dirent));
                    struct dirent *dp2 = malloc(sizeof(struct dirent));
                    while(1){
                        if(readdir_r(dirp, dp1, &dp2) != 0){
                            printf("Fail to get the list of files.\n");
                            break;
                        }
                        if(dp2 == NULL) break;
                        if(dp2->d_name[0] == '.') continue;
                        memcpy(buf + strlen(buf), dp2->d_name, strlen(dp2->d_name)); // 去掉\0
                        // printf("%s\n", dp2->d_name);
                        memcpy(buf + strlen(buf), "\n", 1);
                        payload_len = payload_len + strlen(dp2->d_name) + 1;
                    }
                    // printf("%s", buf);
                    memcpy(buf + strlen(buf), "\0", 1);
                    // printf("%d %d\n", strlen(buf), payload_len);
                    closedir(dirp);
                    free(dp1); free(dp2);
                    set_message(&info_send, protocol, (char)0xA6, unused, 13 + payload_len);
                    // printf("%d\n", info_send.length);
                    write(connfd, (char *)&info_send, sizeof(info_send));
                    write(connfd, buf, payload_len); // if payload_len > MAXLINE???
                }
            }
            // download
            else if(info_recv.type == (char)0xA7){
                memset(buf, 0, sizeof(buf));
                read(connfd, buf, info_recv.length - 13); // read the name of the file
                printf("Get request for the %s file.\n", buf);
                char *path = malloc(strlen("./filedir/") + strlen(buf));
                strcpy(path, "./filedir/");
                strcat(path, buf);

                int fd;
                int payload_len = 0, temp;

                if((fd = open(path, O_RDWR)) != -1){
                    while((temp = read(fd, buf, sizeof(buf))) == MAXLINE){
                        payload_len += temp;
                    }
                    payload_len += temp;
                    printf("the length of the file: %d\n", payload_len);

                    set_message(&info_send, protocol, (char)0xA8, 1, 12);
                    write(connfd, (char *)&info_send, sizeof(info_send));

                    set_message(&info_send, protocol, (char)0xFF, 1, 12 + payload_len);
                    write(connfd, (char *)&info_send, sizeof(info_send));
                    lseek(fd, 0, SEEK_SET); // set the pointer to the beginning
                    while(payload_len > MAXLINE){
                        read(fd, buf, MAXLINE);
                        write(connfd, buf, MAXLINE);
                        payload_len -= MAXLINE;
                    }
                    read(fd, buf, payload_len);
                    write(connfd, buf, payload_len);
                    close(fd);
                }
                else{
                    set_message(&info_send, protocol, (char)0xA8, 0, 12);
                    write(connfd, (char *)&info_send, sizeof(info_send));
                }

                free(path);
            }
            // upload
            else if(info_recv.type == (char)0xA9){
                memset(buf, 0, sizeof(buf));
                read(connfd, buf, info_recv.length - 13);
                char *path = malloc(strlen("./filedir/") + strlen(buf));
                strcpy(path, "./filedir/");
                strcat(path, buf);

                set_message(&info_send, protocol, (char)0xAA, unused, 12);
                write(connfd, (char*) &info_send, sizeof(info_send));

                read(connfd, (char *)&info_recv, sizeof(info_recv));
                if(info_recv.type != (char)0xFF){
                    printf("ERROR with the client FILE_DATA.\n");
                    printf("type: %x\n", info_recv.type);
                }
                int payload_len = info_recv.length - 12;
                printf("the length of the file: %d\n", payload_len);
                int fd = open(path, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
                while(payload_len > MAXLINE){
                    read(connfd, buf, MAXLINE);
                    write(fd, buf, MAXLINE);
                    payload_len -= MAXLINE;
                }
                read(connfd, buf, payload_len);
                write(fd, buf, payload_len);
                printf("Write the file successfully in %s.\n", path);
                close(fd);
                free(path);
            }
            // quit
            else if(info_recv.type == (char)0xAB){
                set_message(&info_send, protocol, (char)0xAC, unused, 12);
                write(connfd, (char*) &info_send, sizeof(info_send));
                close(connfd); state = UNCONNECTED;
                printf("Close the connection to %d.\n", connfd);
                return;
            }
            else printf("Not a command.\n");
        }
    }
    if(n < 0 && errno == EINTR) 
        goto AGAIN;
    else if(n < 0){
        perror("fail to read.\n");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char * argv[]){
    socklen_t clilen;
    int listenfd, connfd;
    struct sockaddr_in cliaddr, servaddr;
    if(argc != 2){
        printf("Usage: myftpserver portnum.\n");
        exit(-1);
    }
    
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); // htol将主机数转换成uint32
    servaddr.sin_port = htons(atoi(argv[1])); // htons将portnum从主机字节序->网络序
    
    bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
    listen(listenfd, LISTENQ);
    for( ; ; ){
        clilen = sizeof(cliaddr);
        int * connfd_p = malloc(sizeof(connfd));
        *connfd_p = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen);
        
        pthread_t a_thread;
        pthread_attr_t a_thread_attr;
        pthread_attr_init(&a_thread_attr);      // 初始化一个线程属性对象
        pthread_create(&a_thread, &a_thread_attr, (void *)&handler, connfd_p);

    }
    return 0;
}

