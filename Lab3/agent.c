#include "routing_simulation.h"

void wrong_command_handler() {
    printf("Wrong form of commands.\n");
    printf("Input 'dv' or 'update:src,dst,dis' or 'show:id' or 'reset:id'\n");
}

int main() {
    // load in the config files first
    struct router_info router_info;
    struct message send_msg;

    if(parse_router_loc(&router_info) == -1) {
        printf("Error when parsing the router location file.\n");
        return 0;
    }

    // establish connection with all the routers
    int sockfd;

    char command[MAXCOMMANDSZ];
    int dis, t;
    uint8_t type;
    uint32_t src, dst;
    uint8_t src_p, dst_p;
    struct rout_table rout_table;
    struct sockaddr_in from;
    socklen_t fromlen;

    while(scanf("%s", command)) {
        // printf("Command: %s\n", command);
        int t = parse_command(command, &type, &src, &dst, &dis);
        src_p = get_index(&router_info, src);
        dst_p = get_index(&router_info, dst);
        if(t == -1){
            wrong_command_handler();
            continue;
        }
        set_message(&send_msg, type, src, dst, dis);
        if(type == DV_COMMAND) {
            for(int i = 0; i < router_info.num; i++) {
                sockfd = set_up_conn(&router_info, i);
                write(sockfd, &send_msg, sizeof(struct message));
                close(sockfd);
                printf("Send dv command to %d.\n", router_info.id[i]);
            }
        }
        else if(type == UPDATE_COMMAND) {
            sockfd = set_up_conn(&router_info, src_p);
            write(sockfd, &send_msg, sizeof(struct message));
            close(sockfd);
            printf("Send update command to %d.\n", router_info.id[src_p]);
            sockfd = set_up_conn(&router_info, dst_p);
            write(sockfd, &send_msg, sizeof(struct message));
            close(sockfd);
            printf("Send update command to %d.\n", router_info.id[dst_p]);
        }
        else if(type == SHOW_COMMAND) {
            sockfd = set_up_conn(&router_info, src_p);
            write(sockfd, &send_msg, sizeof(struct message));
            printf("Send show command to %d.\n", router_info.id[src_p]);
            // need response from the router
            read(sockfd, &rout_table, sizeof(struct rout_table));
            for(int i = 0; i < router_info.num; i++) {
                printf("Dest: %d, next: %d, cost: %d\n", rout_table.id[i], rout_table.next_hop[i], rout_table.dis[i]);
            }
            close(sockfd);
        }
        else if(type == RESET_COMMAND) {
            sockfd = set_up_conn(&router_info, src_p);
            write(sockfd, &send_msg, sizeof(struct message));
            printf("Send reset command to %d.\n", router_info.id[src_p]);
            close(sockfd);
        }
    }

    return 0;
}