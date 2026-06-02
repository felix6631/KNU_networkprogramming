// 2023011844 Kim Hyeon Jin
// Include

#include <netinet/in.h>
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

// streaming thread typedef
typedef struct {
    FILE* fp;
    int block_index;
    int socket;
    int bufsz;
} STREAM;
#define MAX_THREAD 256

void* clnt_handling(void*);
void err_handling(const char*);
void* streaming(void*);


int clnt_cnt;
int clnt_socks[MAX_CLNT];

pthread_mutex_t mutex;
pthread_mutex_t mutex_sum;

int send_bytes;

int main(int argc, char* argv[]) {
    int serv_sock, clnt_sock, clnt_adr_sz=0;
    struct sockaddr_in serv_adr, clnt_adr;
    pthread_t t_id;

    if(argc != 2) {
        printf("Usage: %s <port>\n",argv[0]);
        exit(1);
    }
    //basic info
    printf("----------------------------------------------\n");
    printf("             K-OTT Service Server             \n");
    printf("----------------------------------------------\n");
    
    // thread setting
    pthread_mutex_init(&mutex,NULL);
    pthread_mutex_init(&mutex_sum,NULL);

    //socket setting
    serv_sock = socket(PF_INET,SOCK_STREAM,0);

    //ip addr & port setting
    memset(&serv_adr,0,sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));

    //bind and listen
    if(bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr) == -1)) {
        err_handling("bind err");
    }
    if(listen(serv_sock, 5) == -1) {
        err_handling("listen err");
    }

    //main loop
    while(1) {
        clnt_adr_sz = sizeof(clnt_adr);
        clnt_sock = accept(
            serv_sock,
            (struct sockaddr*)&clnt_adr,
            (socklen_t*)&clnt_adr_sz
        );

        pthread_mutex_lock(&mutex);
        clnt_socks[clnt_cnt++] = clnt_sock;
        pthread_mutex_unlock(&mutex);

        pthread_create(&t_id,NULL,clnt_handling,(void*)&clnt_sock);
        pthread_detach(t_id);
    
        printf("Connected client IP: %s\n",inet_ntoa(clnt_adr.sin_addr));
    }
    close(serv_sock);
    return 0;
}

void err_handling(const char* msg) {
    fprintf(stderr, "%s\n", msg);
    exit(1);
}

void* streaming(void* a) {
    PACKET p;
    int status = 0;
    STREAM* arg = (STREAM*)a;
    status = fseek(arg->fp,arg->block_index*arg->bufsz,SEEK_SET);
    if (status != 0)
        return NULL;
    status = fread(p.buf,sizeof(char),p.len,arg->fp);
    pthread_mutex_lock(&mutex_sum);
    send_bytes += status;
    pthread_mutex_unlock(&mutex_sum);
    return NULL;
}

void* clnt_handling(void* arg) {
    int clnt_sock = *(int*)arg;
    int packet_len = sizeof(PACKET);
    PACKET packet;
    pthread_t stream_thread[256];
    FILE* fp = fopen("hw09.mp4","r");
    int status = 0;
    int bufsz = 0, user_type = 0;
    // read initial request
    status = read(clnt_sock,&packet,packet_len);
    if (packet.command != FILE_REQ) {
        packet.command = COMM_ERR;
        strcpy(packet.buf,"Invalid Request");
        write(clnt_sock,&packet,packet_len);
        goto disconnect_socket;
    }
    else if(status == 0) {
        packet.command = COMM_ERR;
        strcpy(packet.buf,"Failed to Recieve Request; Please reconnect.");
        write(clnt_sock,&packet,packet_len);
        goto disconnect_socket;
    }
    
    switch(packet.type) {
        case USER_BASIC:
            user_type = USER_BASIC;
            bufsz = BASIC_BUF; break;
        case USER_STANDARD:
            user_type = USER_STANDARD;
            bufsz = STANDARD_BUF; break;
        case USER_PREMIUM:
            user_type = USER_PREMIUM;
            bufsz = PREMIUM_BUF; break;
        default:
            packet.command = COMM_ERR;
            strcpy(packet.buf,"Invalid User Type");
            write(clnt_sock,&packet,packet_len);
            goto disconnect_socket;
    }
    packet.len = bufsz;
    packet.command = FILE_SENDING;
    packet.type = user_type;
    
    
    // Each block can be send individually; No need to stream video for single thread.
    // Which means, below code runs 'threads of a thread'
    for(int i=0;i<MAX_THREAD;i++) {
        STREAM s = {.fp = fp, .socket = clnt_sock, .block_index = i, .bufsz = packet.len};
        pthread_create(&stream_thread[i],NULL,streaming,(void*)&s);
        pthread_detach(stream_thread[i]);
    }

    packet.command = FILE_END;
    write(clnt_sock,&packet,packet_len);

    printf(
        "Total Tx bytes: %d to Client %d (%s))\n",
        send_bytes,clnt_sock,
        user_type == USER_BASIC ? "Basic" : user_type == USER_STANDARD ? "Standard" : "Premium"
    );

    read(clnt_sock,&packet,packet_len);
    if (packet.command != FILE_END_ACK) {
        printf("socket closed abnormaly; disconnect.\n");
        goto disconnect_socket;
    }
    
    printf("[Rx] FILE_END_ACK from Client %d => ", clnt_sock);

   

    

disconnect_socket:
    //Client Disconnect
    pthread_mutex_lock(&mutex);
    for(int i=0;i<clnt_cnt;i++) {
        if(clnt_sock == clnt_socks[i]) {
            while(i < clnt_cnt) {
                clnt_socks[i] = clnt_socks[i+1];
                i++;
            }
            break;
        }
    }
    clnt_cnt--;
    pthread_mutex_unlock(&mutex);
    printf("clnt_sock: %d closed.\n",clnt_sock);
    close(clnt_sock);
    
    return NULL;
}
