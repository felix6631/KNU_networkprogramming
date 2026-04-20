//2023011844 Kim HyeonJin
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>

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
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_adr;
    struct sockaddr_in clnt_adr;
    socklen_t clnt_adr_sz;

    if(argc != 2) {
        fprintf(stderr,"usage: %s <port>\n",argv[0]);
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
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));
    if(bind(serv_sock, (struct sockaddr*)&serv_adr,sizeof(serv_adr))== -1)
        err("bind err");

#ifdef DEBUG
    puts("socket binded");
#endif

    if(listen(serv_sock,5) == -1)
        err("listen err");

#ifdef DEBUG
    puts("listening...");
#endif

    clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);
    if(clnt_sock == -1)
        err("clnt_socket err");

#ifdef DEBUG
    puts("client accepted!");
#endif

    // TODO : serve data in order
    REQ_PACKET reqpack;
    RES_PACKET respack;
    int index = 1;
    int success = 0;

#ifdef DEBUG
    puts("enter loop");
#endif

    while(1) {

        read(clnt_sock,&reqpack,sizeof(reqpack));
        printf("[Rx] cmd: %2d, seq: %3d",reqpack.cmd,reqpack.seq);
        if(reqpack.seq == index) {
            //cpy to file_data
            file_data[index-1].seq = index;
            strcpy(file_data[index-1].buf,reqpack.buf);

            //increase nums
            index++; success+=1;

            //set respack
            respack.result = OK;
            respack.cmd = FILE_RES;

            //print sucess
            printf(" --> Success Count: %d",success);
            if(success == MAX_LINE)
                break;
        }
        else {
            respack.result = NOT_OK;
            respack.cmd = FILE_RES;
        }
        respack.seq = reqpack.seq;


        write(clnt_sock,&respack,sizeof(respack));
        printf(" --> [Tx] cmd: %2d, seq: %3d, result: %s\n",respack.cmd,respack.seq,
               respack.result == 1 ? "OK" : "NOT_OK");

    
        
    }

    printf(" --> Reassembled All Data\n");

    for(int i=0;i<50;i++)
        printf("%c",'-');
    puts("");
    for(int i=0;i<MAX_LINE;i++)
        printf("seq [%2d] %s",file_data[i].seq, file_data[i].buf);
    for(int i=0;i<50;i++)
        printf("%c",'-');
    puts("");
    
    respack.cmd = RX_DONE;
    respack.result = OK;
    respack.seq = success;
    write(clnt_sock,&respack,sizeof(respack));
    printf("[Tx] RX_DONE --> ");
    read(clnt_sock,&reqpack,sizeof(reqpack));
    if(reqpack.cmd == RX_DONE_ACK)
        printf("[Rx] RX_DONE_ACK\n");

    close(clnt_sock);
    close(serv_sock);

    printf("Exit Server\n");
    
    return 0;
}

void err(const char* err) {
    fprintf(stderr, "%s\n",err);
    exit(1);
}
