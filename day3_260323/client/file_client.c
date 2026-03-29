#include <stdio.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <memory.h>
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
  if(argc < 3)
    handler("invalid param: halt\n");
  
  int sock;
  struct sockaddr_in addr_serv;

  // step 1. create socket
  sock = socket(PF_INET,SOCK_STREAM,0);
  if(sock == -1)
    handler("socket init failed.\n");
  memset(&addr_serv,0,sizeof(addr_serv));
  addr_serv.sin_family=AF_INET;
  addr_serv.sin_addr.s_addr=inet_addr(argv[1]);
  addr_serv.sin_port=htons(atoi(argv[2]));

  // step 2. connect to server
  if(connect(sock,(struct sockaddr*)&addr_serv,sizeof(addr_serv))==-1)
    handler("connection failed.\n");

  // TCP
  char filename[BUF_SIZE+1] = {0,};
  printf("Input file name: ");
  fgets(filename,BUF_SIZE,stdin);
  
  Packet rdpack = {0,0,0,""};
  Packet wrpack = {0,0,0,""};

  printf("[Client] request %s\n\n",filename);
  strcpy(wrpack.buf,filename);
  write(sock,&wrpack,sizeof(wrpack));
  read(sock,&rdpack,sizeof(rdpack));
  if(strcmp(rdpack.buf,"\033[31mFile Not Found\033[0m")==0) {
    handler("\033[31mFile Not Found\033[0m");
  }
  int recv_bytes = 0;
  int file = open(filename,O_CREAT|O_WRONLY|O_TRUNC);
  while(read(sock,rdpack.buf,sizeof(rdpack))) {
    printf("[Client] Rx SEQ: %d, len: %d bytes\n",rdpack.seq,rdpack.buf_len);
    wrpack.ack = rdpack.seq + rdpack.buf_len + 1;
    recv_bytes += rdpack.buf_len;
    if(rdpack.buf_len < BUF_SIZE)
      break;
    write(sock,&wrpack,sizeof(wrpack));
    printf("[Client] Tx ACK: %d\n",wrpack.ack);
    
  }
  printf("%s recieved (\033[31m%d bytes\033[0m)",filename,recv_bytes);
  
  // close
  printf("Exit client\n");
  close(sock);
  return 0;
}
void handler(char message[]) {
  fputs(stderr,stderr);
  printf("Exit client");
  exit(1);
}
