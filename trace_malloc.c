#define _GNU_SOURCE
#include <dlfcn.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#ifdef __APPLE__
#include <mach/clock.h>
#include <mach/mach.h>
#endif

int fd = 0;
void* (*__malloc)(size_t) = NULL;
void (*__free)(void*) = NULL;
long start_sec = 0, start_mcsec = 0;

inline void get_time(long* sec, long* mcsec) {
  #ifndef __APPLE__
  struct timespec spec;
  clock_gettime(CLOCK_REALTIME, &spec);
  *sec = spec.tv_sec;
  *mcsec = spec.tv_nsec / 1000;
  #else
  clock_serv_t cclock;
  mach_timespec_t mts;
  host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
  clock_get_time(cclock, &mts);
  mach_port_deallocate(mach_task_self(), cclock);
  *sec = mts.tv_sec;
  *mcsec = mts.tv_nsec / 1000;
  #endif
}

void* malloc(size_t size) {
  if (!__malloc) {
    __malloc = (void*(*)(size_t)) dlsym(RTLD_NEXT, "malloc");
  }
  if (!fd) {
    fd = open("malloc.log", O_WRONLY | O_CREAT | O_TRUNC, 0666);
  }
  char record[48];
  long sec, mcsec;
  get_time(&sec, &mcsec);
  if (!start_sec) {
    start_sec = sec;
    start_mcsec = mcsec;
  }
  sec -= start_sec;
  mcsec -= start_mcsec;
  if (mcsec < 0) {
    sec -= 1;
    mcsec += 1000000;
  }
  write(fd, record, sprintf(record, "%ld.%06ld\t%zu\n", sec, mcsec, size));
  return __malloc(size);
}
