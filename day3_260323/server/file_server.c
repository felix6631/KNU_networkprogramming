//2023011844 Kim HyeonJin
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <memory.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#define BUF_SIZE 100
#define SEQ_START 1000
typedef struct {
  int seq;
  int ack;
  int buf_len;
  char buf[BUF_SIZE+1];
} Packet;
void handler(char[]);
int main(int argc, char *argv[]) {
  if(argc < 2)
    handler("invalid param: ./file_server [port]\n");
  
  int sock_serv;
  int sock_clnt;
  int send_bytes = 0;
  struct sockaddr_in addr_serv;
  struct sockaddr_in addr_clnt;
  socklen_t addr_clnt_sz;

  // step 1. create socket
  sock_serv = socket(PF_INET,SOCK_STREAM,0);
  if(sock_serv == -1)
    handler("sock_serv init failed.\n");
  memset(&addr_serv,0,sizeof(addr_serv));
  addr_serv.sin_family = AF_INET;
  addr_serv.sin_addr.s_addr=htonl(INADDR_ANY);
  addr_serv.sin_port = htons(atoi(argv[1]));

#ifdef DEBUG
  puts("socket initialized");
#endif

  // step 2. bind
  if(bind(sock_serv, (struct sockaddr*)&addr_serv,sizeof(addr_serv))==-1)
    handler( "socket address bind failed.\n");

#ifdef DEBUG
  puts("socket address binded");
#endif
  
  // step 3. listen
  if(listen(sock_serv, 5) == -1)
    handler("listen() failed.\n");

#ifdef DEBUG
  puts("socket opened; waiting for client...");
#endif

  printf("------------------------------\n");
  printf("   File Transmission Server\n");
  printf("------------------------------\n");


  addr_clnt_sz = sizeof(addr_clnt);
  
  // step 4. accept
  sock_clnt = accept(sock_serv, (struct sockaddr*)&addr_clnt,&addr_clnt_sz);
  if(sock_clnt == -1)
    handler("socket client accept failed.\n");

#ifdef DEBUG
  puts("clinet accepted");
#endif
  // TCP
  

  Packet rdpack = {0,0,0,""}; memset(&rdpack,0,sizeof(rdpack));
  Packet wrpack = {0,0,0,""}; memset(&wrpack,0,sizeof(wrpack));
  read(sock_clnt,&rdpack,sizeof(rdpack));
  char filename[BUF_SIZE];
  strcpy(filename,rdpack.buf);
  // strip
  if(filename[strlen(filename)-1] == '\n')
    filename[strlen(filename)-1] = '\0';

  // find file and ENOENT exeption 
  int file = open(filename,O_RDONLY);
  if (file == -1) {
    char msg[BUF_SIZE*2];
    sprintf(msg,"\033[31m%s File Not Found.\033[0m\n",filename);
    strcpy(wrpack.buf, "\033[31mFile Not Found\033[0m");
    wrpack.buf_len = strlen(wrpack.buf);
    write(sock_clnt,&wrpack,sizeof(wrpack));
    handler(msg);
  }
  char fileInputBuf[BUF_SIZE]; memset(fileInputBuf,0,BUF_SIZE);
  
  //TCP
  printf("[Server] sending %s\n\n",rdpack.buf);
  wrpack.seq = 1000;
  while(1) { //reading from file
    memset(fileInputBuf,0,BUF_SIZE);
    int fileReadResult = read(file,fileInputBuf,BUF_SIZE);
  #ifdef DEBUG
    if(wrpack.seq > 5000) {
      char dummy;
      fflush(stdin);
      scanf("%c",&dummy);
    }
    printf("read %d: %s\n",fileReadResult,fileInputBuf);
  #endif
  
    strcpy(wrpack.buf,fileInputBuf);
    wrpack.buf_len = strlen(fileInputBuf);
    write(sock_clnt,(Packet*)&wrpack,sizeof(wrpack));
    printf("[Server] Tx: SEQ: %d, %d byte data\n",wrpack.seq,wrpack.buf_len);
    
    send_bytes += wrpack.buf_len;
    if(wrpack.buf_len < 100 || fileReadResult < 0)
      break;
    read(sock_clnt,(Packet*)&rdpack,sizeof(rdpack));
    printf("[Server] Rx ACK: %d\n",rdpack.ack);
    if(rdpack.ack == 0) {
      printf("Client disconected;\n");
      break;
    }
    wrpack.seq = rdpack.ack;
    puts("");
  }
  printf("%s sent (\033[31m %d bytes\033[0m )\n",filename,send_bytes);

    
  // close
  printf("Exit Server\n");
  close(sock_clnt);
  close(sock_serv);
  return 0;
}
void handler(char message[]) {
  fputs(message,stderr);
  printf("Exit server\n");
  exit(1);
}
