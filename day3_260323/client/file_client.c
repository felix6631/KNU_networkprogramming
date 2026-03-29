//2023011844 Kim HyeonJin
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
void handler(const char[]);
int main(int argc, char *argv[]) {
  if(argc < 3)
    handler("invalid param: ./file_client [Address] [Port]\n");
  
  
  int sock;
  struct sockaddr_in addr_serv;

  // step 1. create socket
  sock = socket(PF_INET,SOCK_STREAM,0);
  if(sock == -1)
    handler("socket init failed.\n");
#ifdef DEBUG
  puts("socket initialized.");
#endif

  memset(&addr_serv,0,sizeof(addr_serv));
  addr_serv.sin_family=AF_INET;
  addr_serv.sin_addr.s_addr=inet_addr(argv[1]);
  addr_serv.sin_port=htons(atoi(argv[2]));

  // step 2. connect to server
  if(connect(sock,(struct sockaddr*)&addr_serv,sizeof(addr_serv))==-1)
    handler("connection failed.\n");

#ifdef DEBUG
  puts("server connected");
#endif

  // TCP
  char filename[BUF_SIZE+1] = {0,};
  printf("Input file name: ");
  fgets(filename,BUF_SIZE,stdin);
  if(filename[strlen(filename)-1] == '\n')
    filename[strlen(filename)-1] = '\0';

  
  Packet rdpack = {0,0,0,""}; memset(&rdpack,0,sizeof(rdpack));
  Packet wrpack = {0,0,0,""}; memset(&wrpack,0,sizeof(wrpack));

  printf("[Client] request %s\n\n",filename);
  strcpy(wrpack.buf,filename);
  wrpack.buf_len = strlen(wrpack.buf);

  write(sock,&wrpack,sizeof(wrpack));
#ifdef DEBUG
  puts("initial packet sent");
  printf("wrpack: %d %d %d %s\n",wrpack.seq,wrpack.ack,wrpack.buf_len,wrpack.buf);
#endif

  read(sock,(Packet*)&rdpack,sizeof(rdpack));
#ifdef DEBUG
  puts("initial packet recieved");
  printf("rdpack: %d %d %d %s\n",rdpack.seq,rdpack.ack,rdpack.buf_len,rdpack.buf);
#endif
  if(strcmp(rdpack.buf,"\033[31mFile Not Found\033[0m")==0) {
    handler("\033[31mFile Not Found\033[0m\n");
  }
  int recv_bytes = 0;

  int file = open(filename,O_CREAT|O_TRUNC|O_WRONLY,0644);
  while(1) {
    printf("[Client] Rx SEQ: %d, len: %d bytes\n",rdpack.seq,rdpack.buf_len);
    write(file, rdpack.buf, rdpack.buf_len);
    memset(rdpack.buf,0,BUF_SIZE);
    
    wrpack.ack = rdpack.seq + rdpack.buf_len + 1;
    recv_bytes += rdpack.buf_len;
    if(rdpack.buf_len < BUF_SIZE)
      break;

    //ACK
    write(sock,(Packet*)&wrpack,sizeof(wrpack));
    printf("[Client] Tx ACK: %d\n",wrpack.ack);

    
    if(read(sock,(Packet*)&rdpack,sizeof(rdpack))==-1)
      break;

    puts("");
  }
  printf("%s recieved (\033[31m %d bytes\033[0m ) \n",filename,recv_bytes);
  
  // close
  printf("Exit client\n");
  close(sock);
  return 0;
}
void handler(const char message[]) {
  fputs(message,stderr);
  printf("Exit client\n");
  exit(1);
}
