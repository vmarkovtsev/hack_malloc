#define _GNU_SOURCE
#include <stdio.h>
#include <stddef.h>
#include <dlfcn.h>

#include <unistd.h>
#include <sys/mman.h>

void* malloc(size_t size) {
  printf("malloc\n");
  return ((void*(*)(size_t))dlsym (RTLD_NEXT, "malloc"))(size);
}

void free (void *ptr) {
  printf("free\n");
  ((void(*)(void*))dlsym(RTLD_NEXT, "free"))(ptr);
}
