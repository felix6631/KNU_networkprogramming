// 2023011844 Kim Hyeon JIn
// Include 
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>

// Buffer Sizes
#define BASIC_BUF       10
#define STANDARD_BUF    100
#define PREMIUM_BUF     1000
#define MAX_SIZE PREMIUM_BUF
#define MAX_CLNT        256

// COMMANDS
#define FILE_REQ        0
#define FILE_SENDING    1 
#define FILE_END        2 
#define FILE_END_ACK    3
#define COMM_ERR        -1

// User type
#define USER_BASIC      1 
#define USER_STANDARD   2 
#define USER_PREMIUM    3 

// PACKET typedef
typedef struct {
    int command;
    int type;
    char buf[MAX_SIZE];
    int len;
} PACKET;

pthread_mutex_t mutex;
pthread_mutex_t mutex_sum;

int recv_bytes;

void err_handling(const char*);

int main(int argc, char* argv[]) {
    int sock;
    struct sockaddr_in serv_adr;
    
    if(argc != 3) {
        printf("Usage: %s <ip address> <port>\n",argv[0]);
        exit(1);
    }

    // basic info
    printf("---------------------------------------------\n");
    printf("                K-OTT Service                \n");
    printf("---------------------------------------------\n");

    // mutex setting
    pthread_mutex_init(&mutex,NULL);
    pthread_mutex_init(&mutex_sum,NULL);

    // socket setting
    sock = socket(PF_INET,SOCK_STREAM,0);

    // ip addr & port setting
    memset(&serv_adr,0,sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_adr.sin_port = htons(atoi(argv[2]));

    // connect
    if(connect(sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1) {
        err_handling("connect error");
        exit(1);
    }

    while(1) {
        printf("---------------------------------------------\n");
        printf(" Choose a subscribe type\n");
        printf("---------------------------------------------\n");
        printf("1: Basic, 2: Standard, 3: Premium, 4: quit:  ");
        int user_type;
        scanf("%d", &user_type);
        if(user_type == 4) {
            puts("Exit Program");
            break;
        }
        printf("-----------------------------------------\n");
        printf("1. Download 2. Back to Main Menu: ");

    }

    close(sock);
    return 0;
}  
