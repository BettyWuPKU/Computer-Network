#include "myftp.h"


char protocol[6] = {0xe3, 'm', 'y', 'f', 't', 'p'};
int state = 0;

int read_args(char **argv){
    char c;
    char str[MAX_LEN];
    int cnt1 = 0, cnt2 = 0;
    while(scanf("%c", &c) && c != '\n'){
        if(c == ' '){
            str[cnt2++] = '\0';
            if(argv[cnt1] != NULL) free(argv[cnt1]);
            argv[cnt1] = malloc(cnt2);
            strcpy(argv[cnt1++], str);
            cnt2 = 0;
            memset(str, 0, sizeof(str));
            continue;
        }
        str[cnt2++] = c;
        if(cnt1 > MAX_ARG_NUM){
            printf("Error: too many args\n");
            return -1;
        }
    }
    if(c == '\n'){
        str[cnt2++] = '\0';
        if(argv[cnt1] != NULL) free(argv[cnt1]);
        argv[cnt1] = malloc(cnt2);
        strcpy(argv[cnt1++], str);
    }
    // for(int i = 0; i < cnt1; i++) printf("%s\n", argv[i]);  /////////////////////////
    return cnt1;
}
void free_argp(char **argv){
    for(int i = 0; i < MAX_ARG_NUM; i++){
        if(argv[i]) free(argv[i]);
    }
}

void set_message(struct message_s *info, char protocol[6], char type, char status, int length){
    memcpy(info->protocol, protocol, 6);
    info->type = type;
    info->status = status;
    info->length = length;
}
// set up connection for the TCP and return the sockfd, -1 means failure
int set_connect(){
    int sockfd;
    char *params[MAX_ARG_NUM] = {};
    int port, arg_num, retv;
    struct sockaddr_in servaddr;
    struct message_s info_send, info_recv;

    // input the command: open IP port
    printf("Client> ");
    arg_num = read_args(params);
    // wrong command
    if(strcmp(params[0], "open") != 0){
        printf("ERROR: establish the connection first.\n"); free_argp(params);
        return -1;
    }
    else if(arg_num != 3){
        printf("Usage: open IP + port\n"); free_argp(params);
        return -1;
    }

    // socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    // connect
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons((uint16_t)atoi(params[2]));
    inet_pton(AF_INET, params[1], &servaddr.sin_addr);
    retv = connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    if(retv == -1){
        printf("Connection failed, please try again.\n");
        close(sockfd); free_argp(params);
        return -1;
    }
    else{ printf("Primary connection ok.\n");}
    
    /* connection estabilished */
    // send connect_request
    set_message(&info_send, protocol, (char)0xA1, unused, 12);
    write(sockfd, &info_send, sizeof(info_send));
    
    // receive conncet_reply
    read(sockfd, &info_recv, sizeof(info_recv));
    if(info_recv.type == (char)0xA2 && info_recv.status == 1){
        printf("Server connection accepted.\n");
    }
    else{
        // printf("%x, %d", info_recv.type, info_recv.status);
        printf("Incompatible reply in connection.\n");
        close(sockfd); free_argp(params);
        return -1;
    }
    // change state and go to authentication next
    state = UNAUTH; 
    free_argp(params);
    return sockfd;
}

void auth(int sockfd){
    char *params[MAX_ARG_NUM] = {};
    char buf[MAXLINE];
    int arg_num;
    struct message_s info_send, info_recv;

    printf("Client> ");
    arg_num = read_args(params);
    while(strcmp(params[0], "auth") != 0){
        printf("ERROR: authentication not started. Please issue an authentication command.\n");
        free_argp(params);
        return;
    }
    if(arg_num != 3){
        printf("Usage: auth username password.\n"); free_argp(params);
        return;
    }
    // send auth_request && USER PASS
    memcpy(buf, params[1], strlen(params[1])); // 去掉username末尾的\0
    memcpy(buf + strlen(params[1]), " ", sizeof(" "));
    memcpy(buf + strlen(params[1]) + 1, params[2], strlen(params[2]));
    memcpy(buf + strlen(buf), "\n", sizeof("\n"));
    set_message(&info_send, protocol, (char)0xA3, unused, 13 + strlen(buf));
    write(sockfd, (char *)&info_send, sizeof(info_send));
    write(sockfd, buf, strlen(buf));
    // printf("auth_info:\n%s", buf);

    // receive auth_reply
    read(sockfd, (char *)&info_recv, sizeof(buf));
    if(info_recv.type == (char)0xA4){
        // auth fail
        if(info_recv.status == 0){
            printf("ERROR: Authentication rejected. Connection closed.\n");
            state = UNCONNECTED; close(sockfd); free_argp(params);
            return;
        }
        // auth success
        else if (info_recv.status == 1){
            printf("Authentication granted.\n");
            state = AUTHED;
            free_argp(params);
            return;
        }
    }
    printf("ERROR with the auth reply. Connection closed.\n");
    state = UNCONNECTED; close(sockfd); free_argp(params);
    return;
}

void ls(int sockfd){
    char buf[MAXLINE];
    struct message_s info_send, info_recv;
    int payload_len;
    // send ls_request
    set_message(&info_send, protocol, (char)0xA5, unused, 12);
    write(sockfd, (char *)&info_send, sizeof(info_send));
    // receive ls_reply
    read(sockfd, (char *)&info_recv, sizeof(info_recv));
    printf("%d\n", info_recv.length);
    payload_len = info_recv.length - 13;
    if(info_recv.type != (char)0xA6){
        printf("ERROR with the reply.\n");
        return;
    }
    memset(buf, 0, sizeof(buf));
    printf("----- file list start -----\n");
    read(sockfd, buf, payload_len);
    // int fd_create = open("test_out.txt", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    // write(fd_create, buf, strlen(buf));
    printf("%s", buf);
    printf("----- file list end -----\n");
    return;
}

void download_file(int sockfd, char *file){
    char buf[MAXLINE];
    struct message_s info_send, info_recv;
    int payload_len;

    // GET_REQUEST
    set_message(&info_send, protocol, (char)0xA7, unused, 13 + strlen(file));
    write(sockfd, (char *)&info_send, sizeof(info_send));
    write(sockfd, file, strlen(file));

    // GET_REPLY

    read(sockfd, (char*)&info_recv, sizeof(info_recv));
    if(info_recv.type == (char)0xA8){
        if(info_recv.status == 1){
            printf("Successfully download the file.\n");
        }
        else if(info_recv.status == 0){
            printf("No such file in server.\n"); return;
        }
        else{ printf("ERROR with the reply.\n"); return; }
    }
    else{ printf("ERROR with the reply.\n"); return; }
    // FILE_DATA

    read(sockfd, (char *)&info_recv, sizeof(info_recv));
    if(info_recv.type != (char)0xFF){
        printf("ERROR with the reply.\n"); return;
    }

    payload_len = info_recv.length - 12;
    printf("the length of the file: %d\n", payload_len);
    // 注意权限的设置！！！
    int fd = open(file, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    while(payload_len > MAXLINE){
        read(sockfd, buf, MAXLINE);
        write(fd, buf, MAXLINE);
        payload_len -= MAXLINE;
    }
    read(sockfd, buf, payload_len);
    write(fd, buf, payload_len);

    close(fd);
    return;
}

void upload_file(int sockfd, char *file){
    char buf[MAXLINE];
    struct message_s info_send, info_recv;
    int payload_len = 0, fd, temp;
    if((fd = open(file, O_RDONLY)) == -1){
        printf("This file doesn't exist locally.\n");
        return;
    }
    // PUT_REQUEST
    set_message(&info_send, protocol, (char)0xA9, unused, 13 + strlen(file));
    write(sockfd, (char *)&info_send, sizeof(info_send));
    write(sockfd, file, strlen(file));

    // PUT_REPLY
    read(sockfd, (char *)&info_recv, sizeof(info_recv));
    if(info_recv.type != (char)0xAA){
        printf("ERROR with the reply.\n"); close(fd);
        return;
    }
    // FILE_DATA
    while((temp = read(fd, buf, sizeof(buf))) == MAXLINE){
        payload_len += temp;
    }
    payload_len += temp;
    printf("the length of the file: %d\n", payload_len);
    
    set_message(&info_send, protocol, (char)0xFF, unused, 12 + payload_len);
    write(sockfd, (char *)&info_send, sizeof(info_send));
    lseek(fd, 0, SEEK_SET);
    while(payload_len > MAXLINE){
        read(fd, buf, MAXLINE);
        write(sockfd, buf, MAXLINE);
        payload_len -= MAXLINE;
    }
    read(fd, buf, payload_len);
    write(sockfd, buf, payload_len);

    close(fd);    
    return;
}

void quit_service(int sockfd){
    char buf[MAXLINE];
    struct message_s info_send, info_recv;
    int payload_len;
    // quit_request
    set_message(&info_send, protocol, (char)0xAB, unused, 12);
    write(sockfd, (char *)&info_send, sizeof(info_send));
    // quit_reply
    read(sockfd, (char *)&info_recv, sizeof(info_recv));
    if(info_recv.type != (char)0xAC){
        printf("ERROR with the reply, close failed.\n"); return;
    }

    close(sockfd); state = UNCONNECTED;
}

int handler(){
    int sockfd;
    if(state == UNCONNECTED){
        sockfd = set_connect();
        return 0;
    }
    else if(state == UNAUTH){
        auth(sockfd);
        return 0;
    }
    else if(state == AUTHED){
        char *params[MAX_ARG_NUM] = {};
        int arg_num;

        printf("Client> ");
        arg_num = read_args(params);
        // list files
        if(strcmp(params[0], "ls") == 0){
            if(arg_num != 1) {
                printf("Usage: ls.\n"); free_argp(params);
                return 0;
            }
            ls(sockfd);
        }
        // upload files
        else if(strcmp(params[0], "put") == 0){
            if(arg_num != 2){
                printf("Usage: put file"); free_argp(params);
                return 0;
            }
            upload_file(sockfd, params[1]);
        }
        // download files
        else if(strcmp(params[0], "get") == 0){
            if(arg_num != 2){
                printf("Usage: get file"); free_argp(params);
                return 0;
            }
            download_file(sockfd, params[1]);
        }
        // quit
        else if(strcmp(params[0], "quit") == 0){
            if(arg_num != 1){
                printf("Usage: quit.\n"); free_argp(params);
                return 0;
            }
            quit_service(sockfd); // sockfd closed inside
            free_argp(params);
            return 1;
        }
        // wrong instructions
        else{
            printf("No such instruction.\n");
        }
        free_argp(params);
    }
    return 0;
}

int main(int argc, char * argv[]){
    if(argc != 1){
        printf("Usage: ./myftpclient\n");
        exit(1);
    }
    while(!handler());
    return 0;
}

