//2023011844 Kim HyeonJin
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define BUF_SIZE 100
#define MAX_LINE 10

#define FILE_SEND   1
#define FILE_RES    2
#define RX_DONE     3
#define RX_DONE_ACK 4

#define NOT_OK 0
#define OK     1

typedef struct {
    int seq;
    char buf[BUF_SIZE];
} DATA;

//Client->Server
typedef struct {
    int cmd;
    int seq;
    char buf[BUF_SIZE];
} REQ_PACKET;

//Server->Client
typedef struct {
    int cmd;
    int seq;
    int result;
} RES_PACKET;

DATA file_data[MAX_LINE];

void err(const char* msg);

int main(int argc, char* argv[]) {
    srand(time(NULL));
    int serv_sock;
    struct sockaddr_in serv_adr;

    
    if(argc != 3) {
        fprintf(stderr,"usage: %s <address> <port>\n",argv[0]);
        exit(1);
    }

#ifdef DEBUG
    puts("init");
#endif

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if(serv_sock == -1)
        err("socket err");

#ifdef DEBUG
    puts("socket opened");
#endif

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_adr.sin_port = htons(atoi(argv[2]));

    if(connect(serv_sock,(struct sockaddr*)&serv_adr,sizeof(serv_adr)) == -1)
        err("connect err");

#ifdef DEBUG
    puts("connected to server");
#endif
    
    // TODO : read file, save to file_data
    printf("Read File Data\n");
    for(int i=0;i<50;i++)
        printf("%c",'-');
     puts("");
    FILE* file = fopen("message.txt","r");

    if(file == NULL) {
        err("NO File");
    }
    
    for(int i=0;i<MAX_LINE;i++) {
        fgets(file_data[i].buf,BUF_SIZE,file);
        file_data[i].seq = i+1;
        printf("seq[%2d] %s",file_data[i].seq,file_data[i].buf);
    }
    for(int i=0;i<50;i++)
        printf("%c",'-');
     puts("");

    // TODO : after connect, send random index of file_data
    REQ_PACKET reqpack;
    RES_PACKET respack;
    reqpack.cmd = 1;
#ifdef DEBUG
    puts("enter loop");
#endif

    while(1) {
        int index = rand()%10;
        reqpack.seq = index+1;
        reqpack.cmd = FILE_SEND;
        strcpy(reqpack.buf,file_data[index].buf);

        sleep(1);
        write(serv_sock, (void*)&reqpack, sizeof(reqpack));

        printf("[Tx] cmd: %2d, SEQ: %3d --> ",reqpack.cmd,reqpack.seq);



        read(serv_sock,&respack,sizeof(respack));
        if(respack.cmd== RX_DONE) {
            printf("[Rx] cmd: %2d, SEQ: %3d --> RX_DONE\n",respack.cmd,respack.seq);
            break;
        }
        else
            printf("[Rx] cmd: %2d, SEQ: %3d, result: %s\n",respack.cmd,respack.seq,
               respack.result == 1 ? "OK" : "NOT_OK");

        
    }

    reqpack.cmd = RX_DONE_ACK;
    write(serv_sock,&reqpack, sizeof(reqpack));
    close(serv_sock);
    printf("Exit Client\n");
        
    return 0;
}

void err(const char* msg) {
    fprintf(stderr,"%s\n",msg);
    exit(1);
}
