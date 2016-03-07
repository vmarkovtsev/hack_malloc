#define _GNU_SOURCE
#include <dlfcn.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int fd = 0;
void* (*__malloc)(size_t) = NULL;
void (*__free)(void*) = NULL;

void* malloc(size_t size) {
  if (!__malloc) {
    __malloc = (void*(*)(size_t)) dlsym(RTLD_NEXT, "malloc");
  }
  if (!fd) {
    fd = open("malloc.log", O_WRONLY | O_CREAT, 0666);
  }
  char record[16];
  write(fd, record, sprintf(record, "%zu\n", size));
  return __malloc(size);
}
