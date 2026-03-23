#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
int main(void) {
  int fd = open("data.txt",O_RDONLY);
  if (fd == -1)
    perror("open error");
  printf("fd: %d\n",fd);

  char buffer[128] = {0,};
  int size = read(fd, buffer, sizeof(buffer));
  if (size == -1)
    perror("read error");
  printf("size: %d\n",size);

  puts(buffer);
  
  close(fd);

  return 0;
  
}

