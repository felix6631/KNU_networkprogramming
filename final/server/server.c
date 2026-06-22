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
#include <stdbool.h>
#include <semaphore.h>

// MACRO
#define SEARCH_REQ 1
#define SEARCH_RES 2 
#define SEARCH_END 3 

#define BUF_SIZE 256 
#define KEYWORD_SIZE 50 

// Packet
typedef struct {
    int cmd;
    char keyword[KEYWORD_SIZE];
} REQ_PACKET;

typedef struct {
    int cmd;
    char buf[BUF_SIZE];
} RES_PACKET; 

typedef struct {
    int sock;
    char keyword[KEYWORD_SIZE];
} tt;


// Buffer Sizes
#define MAX_THREAD 256

int clnt_socks[MAX_THREAD];
int clnt_cnt;
void err_handling(const char*);
void* clnt_handling(void*);

void* search1(void*);
void* search2(void*);


pthread_mutex_t mutex;
sem_t sem;

int main(int argc, char* argv[]) {
    int serv_sock, clnt_sock, clnt_adr_sz=0;
    struct sockaddr_in serv_adr, clnt_adr;
    pthread_t t_id;

    if(argc != 2) {
        printf("Usage: %s <port>\n",argv[0]);
        exit(1);
    }
    //basic info
    
    // thread setting
    pthread_mutex_init(&mutex,NULL);
    
    //socket setting
    serv_sock = socket(PF_INET,SOCK_STREAM,0);

    //ip addr & port setting
    memset(&serv_adr,0,sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));

    //bind and listen
    if(bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1) {
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

void* clnt_handling(void* arg) {
    int clnt_sock = *(int*)arg;
    REQ_PACKET req;
    RES_PACKET res;
    memset(&req,0,sizeof(req));
    memset(&res,0,sizeof(res));

    pthread_t t1,t2;
    tt search_arg = {.sock = clnt_sock};
   

    
    // initial read 
    while(1) {
        read(clnt_sock,(void*)&req,sizeof(req));
        if(req.cmd == SEARCH_END) {
            break;
        }


        printf("[Rx] SEARCH_REQ: %s\n",req.keyword);
        
        

        strcpy(search_arg.keyword, req.keyword);
        sem_init(&sem,0,0); 
        printf("----- Search Results -----\n");

        pthread_create(&t1,NULL,search1,(void*)&search_arg);
        pthread_create(&t2,NULL,search2,(void*)&search_arg);
        
        pthread_join(t1,NULL);
        pthread_join(t2,NULL);

        printf("--------------------------\n");
        

        res.cmd = SEARCH_END;
        write(clnt_sock,&res,sizeof(res));
        printf("[Tx] SEARCH_END\n");
        

    }

    printf("Client Disconnected.\n");


    pthread_mutex_lock(&mutex);
    for(int i=0;i<clnt_cnt;i++) {
        if(clnt_sock == clnt_socks[i]) {
            while(i<clnt_cnt) {
                clnt_socks[i] = clnt_socks[i+1];
                i++;
            }
            break;
        }
    }
    clnt_cnt--;
    pthread_mutex_unlock(&mutex);

    close(clnt_sock);


    return NULL;
}

void* search1(void* arg) {

    tt argument = *(tt*)arg;
    FILE* fp = fopen("file1.txt","r");
    char buf[BUF_SIZE] = {0,};
    int linenum = 1;
    RES_PACKET readline;
    memset(&readline,0,sizeof(readline));
    readline.cmd = SEARCH_RES;
    char* result = 0;
    
    while(1) {
        result = fgets(buf,BUF_SIZE,fp);
        if(result == NULL)
            break;
        if(strstr(buf,argument.keyword)) {
            printf("[file1.txt] %2d %s \n",linenum,buf);
            snprintf(readline.buf,BUF_SIZE,"[file1.txt] %2d %s \n",linenum,buf);  
            write(argument.sock,(void*)&readline,sizeof(readline));
        }
        linenum++;
    }

    sem_post(&sem);
    return NULL;
}

void* search2(void* arg) {
    tt argument = *(tt*)arg;
    FILE* fp = fopen("file2.txt","r");
    char buf[BUF_SIZE] = {0,};
    int linenum = 1;
    RES_PACKET readline;
    memset(&readline,0,sizeof(readline));
    readline.cmd = SEARCH_RES;
    char* result = 0;

    sem_wait(&sem);
    
    while(1) {
        result = fgets(buf,BUF_SIZE,fp);
        if(result == NULL)
            break;
        if(strstr(buf,argument.keyword)) {
            printf("[file2.txt] %2d %s \n",linenum,buf);
            snprintf(readline.buf,BUF_SIZE,"[file2.txt] %2d %s \n",linenum,buf); 
            write(argument.sock,(void*)&readline,sizeof(readline));
        }
        linenum++;
    }

    sem_post(&sem);
    return NULL;
}
