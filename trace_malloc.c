#define _GNU_SOURCE
#include <dlfcn.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#ifdef __APPLE__
#include <mach/clock.h>
#include <mach/mach.h>
#endif

#define LOG "malloc.log"

int fd = 0;
void* (*__malloc)(size_t) = NULL;
void (*__free)(void*) = NULL;
long start_sec = 0, start_mcsec = 0;
pthread_mutex_t write_sync = PTHREAD_MUTEX_INITIALIZER;

inline void get_time(long* sec, long* mcsec) {
  long sec_, mcsec_;
  #ifndef __APPLE__
  struct timespec spec;
  clock_gettime(CLOCK_REALTIME, &spec);
  sec_ = spec.tv_sec;
  mcsec_ = spec.tv_nsec / 1000;
  #else
  clock_serv_t cclock;
  mach_timespec_t mts;
  host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
  clock_get_time(cclock, &mts);
  mach_port_deallocate(mach_task_self(), cclock);
  sec_ = mts.tv_sec;
  mcsec_ = mts.tv_nsec / 1000;
  #endif
  if (!start_sec) {
    start_sec = sec_;
    start_mcsec = mcsec_;
  }
  *sec = sec_ - start_sec;
  *mcsec = mcsec_ - start_mcsec;
  if (*mcsec < 0) {
    *sec -= 1;
    *mcsec += 1000000;
  }
}

void* malloc(size_t size) {
  if (!__malloc) {
    __malloc = (void*(*)(size_t)) dlsym(RTLD_NEXT, "malloc");
  }
  if (!fd) {
    fd = open(LOG, O_WRONLY | O_CREAT | O_EXCL, 0666);
    if (fd < 0) {
      return __malloc(size);
    }
  }
  char record[64];
  long sec, mcsec;
  get_time(&sec, &mcsec);
  void* ptr = __malloc(size);
  pthread_mutex_lock(&write_sync);
  write(fd, record, sprintf(record, "%ld.%06ld\t%ld\t%zu\t%p\n",
                            sec, mcsec, pthread_self(), size, ptr));
  pthread_mutex_unlock(&write_sync);
  return ptr;
}

void free (void *ptr) {
  if (!__free) {
    __free = (void(*)(void*)) dlsym(RTLD_NEXT, "free");
  }
  if (!fd) {
    __free(ptr);
    return;
  }
  char record[64];
  long sec, mcsec;
  get_time(&sec, &mcsec);
  pthread_mutex_lock(&write_sync);
  write(fd, record, sprintf(record, "%ld.%06ld\t%ld\t-1\t%p\n",
                            sec, mcsec, pthread_self(), ptr));
  pthread_mutex_unlock(&write_sync);
  __free(ptr);
}