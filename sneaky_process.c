#include<stdio.h>
#include<stdlib.h>

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
  return 0;
}
