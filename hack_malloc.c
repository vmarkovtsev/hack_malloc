#define _GNU_SOURCE
#include <stdio.h>
#include <stddef.h>

#include <unistd.h>
#include <sys/mman.h>

void* malloc(size_t size) {
  write(STDOUT_FILENO, "malloc... ", 10);
  size += sizeof(size_t);
  int page_size = getpagesize();
  int rem = size % page_size;
  if (rem > 0) {
    size += page_size - rem;
  }
  void* addr = mmap(0, size, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
  if (addr == MAP_FAILED) {
    write(STDOUT_FILENO, "fail\n", 5);
    return NULL;
  }
  write(STDOUT_FILENO, "ok\n", 3);
  *(size_t*)addr = size;
  return (size_t*)addr + 1;
}

void free (void *ptr) {
  write(STDOUT_FILENO, "free... ", 8);
  size_t* real_ptr = (size_t*)ptr - 1;
  if (!munmap(real_ptr, *real_ptr)) {
    write(STDOUT_FILENO, "ok\n", 3);
  } else {
    write(STDOUT_FILENO, "fail\n", 5);
  }
}
