#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "sys/socket.h"
#include "netinet/in.h"
#include "arpa/inet.h"
#include "unistd.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "arpa/inet.h"
#include "dirent.h"
#include "pthread.h"
#include "errno.h"

# define MAX_LEN 20
# define MAX_ARG_NUM 5
# define MAXLINE 1024
# define LISTENQ 64
enum status{
    unused = '2', success = '1', fail = '0'
};
enum state_code{
    UNCONNECTED = 0, UNAUTH = 1, AUTHED = 2
};
struct message_s {
    char protocol[6];       // protocol magic number
    char type;              // type
    char status;            // status
    int length;             // length (header + payload)
}__attribute__ ((packed));