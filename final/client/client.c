// 2023011844 Kim Hyeon Jin
// Include 
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>

// Buffer Sizes
#define SEARCH_REQ 1
#define SEARCH_RES 2 
#define SEARCH_END 3 

#define BUF_SIZE 256 
#define KEYWORD_SIZE 50 

typedef struct {
    int cmd;
    char keyword[KEYWORD_SIZE];
} REQ_PACKET;

typedef struct {
    int cmd;
    char buf[BUF_SIZE];
} RES_PACKET;

pthread_mutex_t mutex;


void err_handling(const char*);

int main(int argc, char* argv[]) {
    int sock;
    struct sockaddr_in serv_adr;
    char input_buf[BUF_SIZE];
    REQ_PACKET request; 
    RES_PACKET result;
    if(argc != 3) {
        printf("Usage: %s <ip address> <port>\n",argv[0]);
        exit(1);
    }

    // basic info

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
    
    while(1) {
        printf("Type keyword('quit' to exit): ");
        scanf("%s",input_buf);
        if(strncmp(input_buf,"quit",4) == 0) {
            break;
        }
        
        strcpy(request.keyword,input_buf);
        request.cmd = SEARCH_REQ;
        write(sock,(void*)&request,sizeof(request));
        

        printf("----- Search Results ----- \n");
        while(1) {
            int str_len = read(sock,(void*)&result,sizeof(result));
            if(result.cmd == SEARCH_END) {
                break;
            }
            result.buf[str_len] = 0;
            fputs(result.buf,stdout);
        }
        printf("--------------------------\n");
        
    } // while
    strcpy(request.keyword,"quit");
    request.cmd = SEARCH_END;
    write(sock,(void*)&request,sizeof(request));
    printf("[Tx] SEARCH_END\n");

    printf("Exit Client Program!");
    close(sock);
    return 0;
}  

void err_handling(const char* msg) {
    fprintf(stderr,"%s\n",msg);
    exit(1);
}


