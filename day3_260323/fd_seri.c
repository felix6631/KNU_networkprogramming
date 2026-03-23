#include <stdio.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

int main(void) {
  int fd[3] = {0,};

  fd[0] = socket(PF_INET, SOCK_STREAM, 0); //TCP protocol
  fd[1] = open("test.dat", O_CREAT|O_WRONLY|O_TRUNC, 0644);
  fd[2] = socket(PF_INET, SOCK_DGRAM, 0); //UDP protocol

  for(size_t i = 0; i < 3; i++) {
    printf("fd %ld: %d\n",i+1,fd[i]);
  }

  for(size_t i = 0; i < 3; i++) {
    close(fd[i]);
  }

  return 0;
}
