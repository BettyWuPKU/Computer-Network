#include "routing_simulation.h"

struct router_info router_info;
struct router_proto router;
long long DV_counter = 0;

void router_error(char *str) {
    printf("%s", str);
    exit(EXIT_FAILURE);
}

void work(int sockfd) {
    struct message recv_msg;
    struct message send_msg;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in from;
    socklen_t fromlen;
    uint32_t dv_cnt = 0;
    int n;

    // recvfrom(sockfd, (void *)test_recv, MAXLINESZ, 0, (struct sockaddr *)&from, &fromlen);
    // printf("receive: %s\n", test_recv);

    // start working to receive commands from the agent
    while(1) {
        if((n = read(sockfd, &buffer, BUFFER_SIZE)) < 0) {
            printf("read error!\n");
            continue;
        }
        if(n == 0) continue;
        memcpy(&recv_msg, buffer, sizeof(recv_msg));
        // propogate the cost table
        if(recv_msg.type == DV_COMMAND) {
            DV_counter++;
            printf("Received dv command.\n");
            int sendfd = 0;
            for(int i = 0; i < router_info.num; i++) {
                if(router.id == router_info.id[i]) continue;
                sendfd = set_up_conn(&router_info, i);
                set_message(&send_msg, ROUTER_DV, router.id, router_info.id[i], 0);
                memcpy(buffer, &send_msg, sizeof(send_msg));
                memcpy(buffer + sizeof(send_msg), &router.cost_table, sizeof(struct cost_table));
                write(sockfd, buffer, sizeof(struct message) + sizeof(struct cost_table));
                close(sendfd);
                printf("Send dv vectors to router %d.\n", router_info.id[i]);
            }
        }
        // update certain cost
        else if(recv_msg.type == UPDATE_COMMAND) {
            printf("Received update command.\n");
            router.cost_table.cost[get_index(&router_info, recv_msg.dst)] = recv_msg.dis;
        }
        // send back the 
        else if(recv_msg.type == SHOW_COMMAND) {
            printf("Received show command.\n");
            memcpy(buffer, &router.rout_table, sizeof(struct rout_table));
            write(sockfd, buffer, sizeof(struct rout_table));

        }
        else if(recv_msg.type == RESET_COMMAND) {
            printf("Received reset command.\n");
            DV_counter = 0;
        }
        else if(recv_msg.type == ROUTER_DV) {
            printf("Received message from router %d.\n", recv_msg.src);
            struct cost_table temp;
            int index = get_index(&router_info, recv_msg.src);
            memcpy(&temp, buffer + sizeof(struct message), sizeof(struct cost_table));
            if(diff_and_update_cost(&router.neighbor_DV, &temp, router_info.num, index)) {
                update_rout_table(&router, router_info.num);
            }
            break;
        }
        else {
            printf("Invalid type of command.\n");
        }
        break;
    }
    close(sockfd);

}


int main(int argc, char **argv) {
    if(argc != 2) {
        router_error("Usage: ./router [id].\n");
    }
    int n;

    sscanf(argv[1], "%d", &router.id);
    // read in the configuration files
    if(parse_router_loc(&router_info) == -1) 
        router_error("Error when parsing the router location file.\n");
    if(parse_topology(&n, &router.neighbor_DV, &router_info) == -1) 
        router_error("Error when parsing the topology configuration file.\n");
    // initialize its own info
    init_router(&router, &router_info);

    // receive test data from the agent
    socklen_t socklen;
    int listenfd = 0, sockfd = 0;
    struct sockaddr_in listen_addr, address;
    if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
        router_error("socket failed.\n");

    printf("current ip: %s, port:%d, id:%d\n", router.ip, router.port, router.id);
    memset(&listen_addr, 0, sizeof(struct sockaddr_in));
    memset(&address, 0, sizeof(struct sockaddr_in));
    listen_addr.sin_family = AF_INET;
    listen_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    listen_addr.sin_port = htons(router.port);

    if(bind(listenfd, (struct sockaddr *) &listen_addr, sizeof(listen_addr)) < 0)
        router_error("bind failed.\n");
    listen(listenfd, ROUTER_NUM);

    struct message recv_msg;
    while(1) {
        socklen = sizeof(address);
        sockfd = accept(listenfd, (struct sockaddr *) &address, &socklen);
        printf("Connection built.\n");
        work(sockfd);
    }



    return 0;
}