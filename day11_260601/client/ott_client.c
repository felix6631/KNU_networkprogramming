// 2023011844 Kim Hyeon Jin
// ott_client.c 2026/06/02 ~ 2026/06/03
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

// main thread typedef
typedef struct {
    int socket;
    int user_type;
} main_thread_t;



pthread_mutex_t mutex;


void err_handling(const char*);
void* recv_vid(void*);

int main(int argc, char* argv[]) {
    int sock,user_type,select_dummy;
    struct sockaddr_in serv_adr;
    pthread_t rcv_thread;
    main_thread_t mtt;
    
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
    
    // mtt setting
    mtt.socket = sock;

    while(1) {
main_menu:
        printf("---------------------------------------------\n");
        printf(" Choose a subscribe type\n");
        printf("---------------------------------------------\n");
        printf("1: Basic, 2: Standard, 3: Premium, 4: quit:  ");
        
        scanf("%d", &user_type);
        switch(user_type) {
            case 1:
                mtt.user_type = USER_BASIC; break;
            case 2:
                mtt.user_type = USER_STANDARD; break;
            case 3:
                mtt.user_type = USER_PREMIUM; break;
            case 4:
                puts("Exit Program");
                exit(0);
            default:
                puts("Invalid option. Try Again.\n");
                goto main_menu;
        }
download:
        printf("-----------------------------------------\n");
        printf("1. Download 2. Back to Main Menu: ");
        scanf("%d",&select_dummy);
        if(select_dummy == 2)
            goto main_menu;
        else if(select_dummy == 1)
            break;
        else {
            printf("Invalid option. Try Again.\n");
            goto download;
        }
    } // while

    // Recieve
    pthread_create(&rcv_thread,NULL, recv_vid, (void*)&mtt);

    pthread_join(rcv_thread,NULL);
    

    close(sock);
    return 0;
}  

void err_handling(const char* msg) {
    fprintf(stderr,"%s\n",msg);
    exit(1);
}

void* recv_vid(void* arg) {
    main_thread_t mtt = *(main_thread_t*)arg;
    PACKET p;
    int total_recv = 0;
    struct timespec start, end;

    // send FILE_REQ
    p.command = FILE_REQ;
    p.type = mtt.user_type;

    memset(p.buf, 0, MAX_SIZE);
    switch(p.type) {
        case USER_BASIC:    
            p.len = BASIC_BUF;    break;
        case USER_STANDARD: 
            p.len = STANDARD_BUF; break;
        case USER_PREMIUM:  
            p.len = PREMIUM_BUF;  break;
    }
    write(mtt.socket, &p, sizeof(p));

    // start timer
    clock_gettime(CLOCK_MONOTONIC, &start);

    // recv loop
    while(1) {
        read(mtt.socket, &p, sizeof(p));
        if(p.command == FILE_END) break;
        if(p.command == FILE_SENDING) {
            total_recv += p.len;
            printf(".");
            fflush(stdout);
        }
    }

    // 타이머 종료
    clock_gettime(CLOCK_MONOTONIC, &end);
    long msec = (end.tv_sec - start.tv_sec) * 1000
              + (end.tv_nsec - start.tv_nsec) / 1000000;

    // FILE_END_ACK 전송
    p.command = FILE_END_ACK;
    write(mtt.socket, &p, sizeof(p));

    printf("\nFile Transmission Finished\n");
    printf("Total received bytes: %d\n", total_recv);
    printf("Downloading time: %ld msec\n", msec);
    printf("Client closed\n");

    return NULL;
}
