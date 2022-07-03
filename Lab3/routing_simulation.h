#include "stdio.h"
#include "stdlib.h"
#include "fcntl.h"
#include "string.h"
#include "arpa/inet.h"
#include "sys/socket.h"
#include "netinet/in.h"
#include "unistd.h"


#define ROUTER_NUM 10
#define MAXLINESZ 128
#define MAXDIS 0x7fffffff
#define MAXCOMMANDSZ 10
#define BUFFER_SIZE 1024

enum COMMAND {DV_COMMAND, UPDATE_COMMAND, SHOW_COMMAND, RESET_COMMAND, ROUTER_DV};
// the router location file looks like this:
// total number of routers (no more than 10)
// router_ip1, port1, router_id1 (not consecutive, are integers and unique)
// ...
char router_loc_filename[] = "router_loc.txt";
// the topology configuration file looks like this
// 
char topology_config_filename[] = "topology_config.txt";

// message between the agent and a router
// agent will send this to a router
// format: dv; update: 1, 2, 3; show: 1; reset: 2;
struct message {
    uint8_t type;   // 0 dv; 1 update; 2 show; 3 reset; 4 from router
    // uint32_t checksum;
    uint32_t src;   // only not used by dv or give the id of the sender
    uint32_t dst;   // only used by update
    int dis;   // only used by update
};

// the direct cost from current router to other router
// router_id |  1  |  5  |  3  |
// cost      |  10 |  8  |  -1 |
struct cost_table {
    uint32_t id[ROUTER_NUM];
    int cost[ROUTER_NUM];
};
// dest router id  |  next hop  |   dis  ｜
//      1          ｜   5       ｜    3   ｜
//      5          ｜   5       ｜    8   ｜
//      3          ｜   1       ｜    13   ｜
struct rout_table {
    uint32_t id[ROUTER_NUM];
    int next_hop[ROUTER_NUM];
    int dis[ROUTER_NUM];
};

// the basic info of all the routers
struct router_info {
    int num;
    char ip[ROUTER_NUM][MAXLINESZ];
    uint32_t port[ROUTER_NUM];
    uint32_t id[ROUTER_NUM];
};
// the overall info of the system
struct neighbor_DV {
    uint32_t id[ROUTER_NUM];
    int dv[ROUTER_NUM][ROUTER_NUM];
};

struct router_proto {
    char ip[MAXLINESZ];
    uint32_t port;
    uint32_t id;                    // info get from the router location file
    struct cost_table cost_table;
    struct rout_table rout_table;   
    struct neighbor_DV neighbor_DV; // info used to accomplish routing algorithmn
};

int set_message(struct message *msg, uint8_t type, uint32_t src, uint32_t dst, int dis);
uint8_t get_index(struct router_info* info, uint32_t id);
int parse_router_loc(struct router_info* info);
int parse_topology(int *num, struct neighbor_DV* DV, struct router_info* router_info);
int init_router(struct router_proto* router, struct router_info* router_info);
void update_rout_table(struct router_proto* router, uint32_t num);
int diff_and_update_cost(struct neighbor_DV* DV, struct cost_table* cost, uint32_t num, uint32_t id);
int set_up_conn(struct router_info* router_info,int index);
int parse_notes(char * str, int len, char c);
int parse_command(char *command, uint8_t *type, uint32_t *src, uint32_t*dst, int *dis);

// return 0 if success, otherwise -1
int set_message(struct message *msg, uint8_t type, uint32_t src, uint32_t dst, int dis) {
    msg->type = type;
    msg->src = src; 
    msg->dst = dst; 
    msg->dis = dis;
    // msg->checksum = 0;
    // char buffer[BUFFER_SIZE];
    // memcpy((void *)buffer, msg, sizeof(struct message_a_r));
    // msg->checksum = compute_checksum((void *)buffer,sizeof(struct message_a_r));
    return 0;
}

// return the index of specific id; return -1 if not found
uint8_t get_index(struct router_info* info, uint32_t id) {
    for(int i = 0; i < info->num; i++) {
        if(info->id[i] == id) 
            return i;
    }
    return -1;
}

// read in the router location file
// return -1 on failure, 0 on success
int parse_router_loc(struct router_info* info) {
    // int fd = open(router_loc_filename, O_RDWR);
    FILE *f = 0;
    if((f = fopen(router_loc_filename, "r")) == 0) {
        printf("Fail to open the router location file.\n");
        return -1;
    }
    int cnt = 0;
    char buffer[MAXLINESZ];
    fscanf(f, "%d\n", &info->num);
    while(cnt < info->num){
        fscanf(f, "%s", buffer);
        int t = 0;
        while(buffer[t++] != ',');
        buffer[t - 1] = '\0';
        sscanf(buffer, "%s", info->ip[cnt]);
        sscanf(buffer + t, "%d,%d", &info->port[cnt], &info->id[cnt]);
        printf("Read in the ip: %s  port: %d  id: %d\n", info->ip[cnt], info->port[cnt], info->id[cnt]);
        cnt++;
    }
    if(cnt != info->num) {
        printf("Error! %d lines of info given but %d need to be provided.\n", cnt, info->num);
        return -1;
    }
    return 0;
}

// read in the topology configuration file
// return -1 on failure, 0 on success
int parse_topology(int *num, struct neighbor_DV* DV, struct router_info* router_info) {
    // int fd = open(router_loc_filename, O_RDWR);
    FILE *f = 0;
    if((f = fopen(topology_config_filename, "r")) == 0) {
        printf("Fail to open the topology configuration file.\n");
        return -1;
    }
    memset(DV->dv, -1, sizeof(DV->dv));
    int cnt = 0;
    int src, dst, dis;
    fscanf(f, "%d\n", num);
    while(cnt < *num) {
        fscanf(f, "%d,%d,%d", &src, &dst, &dis);
        DV->dv[get_index(router_info, src)][get_index(router_info, dst)] = dis;
        printf("Read in the src: %d  dst: %d  distance: %d\n", src, dst, dis);
        cnt++;
    }
    for(int i = 0; i < *num; i++)
        DV->dv[i][i] = 0;
    if(cnt != *num) {
        printf("Error! %d lines of info given but %d need to be provided.\n", cnt, *num);
        return -1;
    }
    return 0;
}


int init_router(struct router_proto* router, struct router_info* router_info) {
    // init basic info (ip/id/port in router)
    int i;
    for(i = 0; i < router_info->num; i++) {
        if(router_info->id[i] == router->id) {
            router->port = router_info->port[i];
            memcpy(router->ip, router_info->ip[i], sizeof(router_info->ip[i]));
            break;
        }
    }
    if(i == router_info->num){
        printf("id not found\n");
        return -1;
    }
    uint8_t src_p = i;
    struct neighbor_DV *DV = &router->neighbor_DV;
    // init the cost table
    for(int i = 0; i < router_info->num; i++) {
        router->cost_table.cost[i] = DV->dv[src_p][i];
    }
    // init the rout table
    memset(router->rout_table.next_hop, -1, sizeof(router->rout_table.next_hop));
    for(int i = 0; i < router_info->num; i++) {
        router->rout_table.id[i] = router_info->id[i];
        if(router->cost_table.cost[i] != -1){
            router->rout_table.dis[i] = router->cost_table.cost[i];
            router->rout_table.next_hop[i] = router->rout_table.id[i];
        }
        else
            router->rout_table.dis[i] = -1;
    }
    update_rout_table(router, router_info->num);

    for(int i = 0; i < router_info->num; i++) {
        printf("Dest: %d, next hop: %d, dis: %d\n", router->rout_table.id[i], router->rout_table.next_hop[i], router->rout_table.dis[i]);
    }
    return 0;
}
// return 1 if diff, otherwise 0 is returned

void update_rout_table(struct router_proto* router, uint32_t num) {
    struct neighbor_DV* DV = &router->neighbor_DV;
    for(int k = 0; k < num; k++)
        for(int i = 0; i < num; i++)
            for(int j = 0; j < num; j++) {
                if(router->rout_table.dis[j] != -1 && DV->dv[j][i] != -1)
                    if(router->rout_table.dis[i] > router->rout_table.dis[j] + DV->dv[j][i]) {
                        router->rout_table.dis[i] = router->rout_table.dis[j] + DV->dv[j][i];
                        router->rout_table.next_hop[i] = router->rout_table.id[j];
                    }
            }
}

int diff_and_update_cost(struct neighbor_DV* DV, struct cost_table* cost, uint32_t num, uint32_t id) {
    int retv = 0;
    for(int i = 0; i < num; i++) 
        if(DV->dv[id][i] != cost->cost[i]){
            DV->dv[id][i] = cost->cost[i];
            retv = 1;
        }
    return retv;
}

// give the consecutive virtual index of dest
int set_up_conn(struct router_info* router_info,int index) {
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    int sockfd = 0;
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_port = htons(router_info->port[index]);
    if(inet_pton(AF_INET, router_info->ip[index], &address.sin_addr) <= 0) {
        perror("address failed.");
        exit(EXIT_FAILURE);
    }
    connect(sockfd, (struct sockaddr *) &address, sizeof(struct sockaddr));
    // write(sockfd[i], test_str, sizeof(test_str));
    printf("Set up connection with %d\n", router_info->id[index]);
    return sockfd;
}

// return the place that specific note first appear
// return -1 otherwise
int parse_notes(char * str, int len, char c) {
    for(int i = 0; i < len; i++) {
        if(str[i] == c) return i;
    }
    return -1;
}

int parse_command(char *command, uint8_t *type, uint32_t *src, uint32_t*dst, int *dis) {
    if(!strcmp(command, "dv")) {
        *type = DV_COMMAND;
        *src = 0;
        *dst = 0;
        *dis = 0;
        return 0;
    }
    int t = parse_notes(command, strlen(command), ':');
    if(t < 0) return -1;
    if(strstr(command, "update")) {
        *type = UPDATE_COMMAND;
        int error_no = sscanf(command, "update:%d,%d,%d", src, dst, dis);
        if(error_no == -1) return -1;
        return 0;
    }
    if(strstr(command, "show")) {
        *type = SHOW_COMMAND;
        int error_no = sscanf(command, "show:%d", src);
        if(error_no == -1) return -1;
        *dst = 0; *dis = 0;
        return 0;
    }
    if(strstr(command, "reset")) {
        *type = RESET_COMMAND;
        int error_no = sscanf(command, "reset:%d",src);
        if(error_no == -1) return -1;
        *dst = 0; *dis = 0;
        return 0;
    }
    return -1;
}