#define _GNU_SOURCE
#include <stdio.h>
#include <stddef.h>
#include <dlfcn.h>
#include <unistd.h>

void* malloc(size_t size) {
  write(STDOUT_FILENO, "malloc\n", 7);
  // printf("malloc\n"); leads to Segmentation Fault on MacOSX
  return ((void*(*)(size_t))dlsym (RTLD_NEXT, "malloc"))(size);
}

void free (void *ptr) {
  write(STDOUT_FILENO, "free\n", 5);
  ((void(*)(void*))dlsym(RTLD_NEXT, "free"))(ptr);
}
