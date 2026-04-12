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
  int cmd;   // GAME_REQ    
  char ch;   // Send one alphabet
} REQ_PACKET;

// Request Packet : Server -> Client
typedef struct {
  int cmd;   // GAME_RES, GAME_END
  char board[BOARD_SIZE][BOARD_SIZE]; // Send only corrent alphabets
  int result;
} RES_PACKET;

void printboard(char[][BOARD_SIZE], char[][BOARD_SIZE]);

int main(int argc, char *argv[]) {
  srand(time(NULL));
  char board_round[BOARD_SIZE][BOARD_SIZE] = {0,};
  REQ_PACKET req;
  RES_PACKET res;
  memset(&req, 0, sizeof(req));
  memset(&res, 0, sizeof(res));

  // board init
  printf("----------------------------------\n");
  printf("   Finding Alphabet Game Server   \n");
  printf("----------------------------------\n");
  for(int i=0;i<BOARD_SIZE;i++) {
    for(int j=0;j<BOARD_SIZE;j++) {
       board_round[i][j] = rand()%26 + 'A';
       res.board[i][j] = ' ';
    }
  }
  printboard(board_round,res.board);
  
  // socket setting
  int serv_sock;
  socklen_t clnt_adr_sz; // client_address_size
  struct sockaddr_in serv_adr, clnt_adr;

  if(argc!=2) {
    fprintf(stderr, "Usage : %s <port> \n", argv[0]);
  }

  // create UDP socket
  serv_sock = socket(PF_INET, SOCK_DGRAM, 0);
  if(serv_sock== -1) {
    fprintf(stderr, "UDP Socket creation error\n");
    exit(1);
  }

  memset(&serv_adr,0, sizeof(serv_adr));
  serv_adr.sin_family=AF_INET;
  serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
  serv_adr.sin_port=htons(atoi(argv[1]));

  // socket bind
  if(bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr))== -1) {
    fprintf(stderr, "Bind error\n");
    exit(1);
  }

  // game start!
  while(1) {
    clnt_adr_sz = sizeof(clnt_adr);
    recvfrom(serv_sock, &req, sizeof(req), 0, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);
    printf("[Server] Rx cmd = %d, ch = %c\n",req.cmd,req.ch);
    
    // compare
    for(int i=0;i<BOARD_SIZE;i++) {
      for(int j=0;j<BOARD_SIZE;j++) {
        if(board_round[i][j] == req.ch) {
          res.board[i][j] = board_round[i][j];
          res.result += 1;
        }
      }
    }
     
    if(res.result == 25) {
      res.result = 0;
      res.cmd = GAME_END;
    }
    else {
      res.cmd = GAME_RES;
    }

    sendto(serv_sock, &res, sizeof(res), 0, (struct sockaddr*)&clnt_adr,clnt_adr_sz);
    printf("[Server] Tx cmd = %d, result = %d\n",res.cmd,res.result);

    if(res.cmd == GAME_END) {
      printf("No empty space. Exit this program.\n");
      break;
    }
    
    // print
    printboard(board_round, res.board);
  }

  close(serv_sock);
  printf("Exit Server Program\n");  
  return 0;
}


void printboard(char board1[][BOARD_SIZE], char board2[][BOARD_SIZE]) {

  for(int i=0;i<5;i++) {
    printf("+-------------------+\t");    printf("+-------------------+\n");
    printf("|   |   |   |   |   |\t");    printf("|   |   |   |   |   |\n");
    for(int j=0;j<5;j++) {
      printf("| %c ",board1[i][j]);
    }
    printf("\t");
    for(int j=0;j<5;j++) {
      printf("| %c ",board2[i][j]);
    }
    printf("|   |   |   |   |   |\t");    printf("|   |   |   |   |   |\n");
  }
    printf("+-------------------+\t");    printf("+-------------------+\n");
}
