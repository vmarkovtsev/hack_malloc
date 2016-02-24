#define _GNU_SOURCE
#include <stdio.h>
#include <stddef.h>

#include <unistd.h>
#include <sys/mman.h>

typedef struct {
  void* addr;
  size_t size;
} MemoryInfo;

static MemoryInfo infos[100];

static int info_pos = 0;

void* malloc(size_t size) {
  printf("malloc\n");
  if (info_pos >= sizeof(infos) / sizeof(MemoryInfo)) {
    return NULL;
  }
  int page_size = getpagesize();
  int rem = size % page_size;
  if (rem > 0) {
    size += page_size - rem;
  }
  void* addr = mmap(0, size, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
  infos[info_pos].addr = addr;
  infos[info_pos].size = size;
  info_pos++;
  return addr;
}

void free (void *ptr) {
  printf("free... ");
  for (int i = info_pos - 1; i >= 0; i--) {
    if (infos[i].addr == ptr) {
      printf("ok");
      munmap(ptr, infos[i].size);
      break;
    }
  }
  printf("\n");
}
