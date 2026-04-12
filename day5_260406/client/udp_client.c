// 2023011844 Kim HyeonJin
// include
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>

// cmd type
#define BOARD_SIZE 5
#define GAME_REQ 1
#define GAME_RES 2
#define GAME_END 3

// Request Packet : Client -> Server
typedef struct {
  int cmd;    // GAME_REQ
  char ch;    // Send one alphabet
} REQ_PACKET;

// Result Packet : Server -> Client
typedef struct {
  int cmd;    // GAME_RES, GAME_END
  char board[BOARD_SIZE][BOARD_SIZE]; // Send only correct alphabets
  int result;
} RES_PACKET;

void printboard(char[][BOARD_SIZE]);

int main(int argc, char* argv[]) {
  srand(time(NULL));
  int sock;
  REQ_PACKET req;
  RES_PACKET res;
  memset(&req, 0, sizeof(req));
  memset(&res, 0, sizeof(res));
  socklen_t adr_sz;
  struct sockaddr_in serv_adr, from_adr;

  if (argc!=3) {
    fprintf(stderr,"Usage : %s <IP> <port>\n",argv[0]);
    exit(1); 
  }

  // init
  printf("----------------------------------\n");
  printf("   Finding Alphabet Game Client   \n");
  printf("----------------------------------\n");

  sock = socket(PF_INET, SOCK_DGRAM, 0);
  if(sock == -1) {
    fprintf(stderr,"socket error\n");
  }

  memset(&serv_adr, 0, sizeof(serv_adr));
  serv_adr.sin_family = AF_INET;
  serv_adr.sin_addr.s_addr = inet_addr(argv[1]);
  serv_adr.sin_port = htons(atoi(argv[2]));

  while(1) {
    
  }  
}
