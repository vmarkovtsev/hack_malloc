#include <stdio.h>
#include <stdlib.h>

int main() {
  printf("start\n");
  void* mem = malloc(100);
  printf("malloc() -> %p\n", mem);
  *(int*)mem = 0;
  free(mem);
  printf("end\n");
  return 0;
}
