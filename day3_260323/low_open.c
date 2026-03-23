#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main(void) {
  int fd = open("data.txt", O_CREAT|O_WRONLY|O_TRUNC, 0644);
  //fd 0: stdin, 1: stodut, 2: stderr
  char buf[] = "Let's go!\n";

  if (fd == -1)
    perror("open error");

  printf("fd: %d\n",fd);
  int size = write(fd, buf, sizeof(buf));
  //size: saved bytes
  printf("write size: %d\n",size);
  if(size == -1)
    perror("write error");

  close(fd);
  
  
  return 0;
}

