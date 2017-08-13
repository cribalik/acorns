
/** HEADER **/

#ifndef THREAD_H
#define THREAD_H

#if !defined(__linux__) && !defined(__APPLE__) && !defined(_MSC_VER)
#error Unknown platform
#endif

#if defined(__linux__)

#include <pthread.h>
typedef pthread_t Thread;
typedef pthread_mutex_t Thread_Mutex;
#define THREAD_PROC(name, arg) void* name(void *arg)

#if _XOPEN_SOURCE >= 500 || _POSIX_C_SOURCE >= 199309L
#define THREAD__HAS_USLEEP
#endif

#else
#error Unimplemented platform
#endif

int thread_create(Thread *thread, THREAD_PROC(proc, arg), void *args);
int thread_join(Thread thread);
int thread_mutex_init(Thread_Mutex *mutex);
int thread_sleep(int seconds);
#ifdef THREAD__HAS_USLEEP
int thread_usleep(int seconds, int milliseconds);
#endif

#endif

/** IMPLEMENTATION **/

#ifdef THREAD_IMPLEMENTATION

/** LINUX **/

#if defined(__linux__)

#include <stdlib.h>
#include <unistd.h>
#include <time.h>

struct ArgWrapper {void *arg; void(*proc)(void*);};
void *thread__pthread_create_wrapper(void *arg);

int thread_create(Thread *thread, THREAD_PROC(proc, arg), void *arg) {
  return pthread_create(thread, 0, proc, arg);
}

int thread_join(Thread thread) {
  return pthread_join(thread, 0);
}

#define thread_mutex_init(mutex_ptr) pthread_mutex_init((mutex_ptr), 0)
#define thread_lock(mutex_ptr) pthread_mutex_lock(mutex_ptr)
#define thread_unlock(mutex_ptr) pthread_mutex_unlock(mutex_ptr)

int thread_sleep(int seconds) {
  return sleep(seconds);
}

#ifdef THREAD__HAS_USLEEP
int thread_usleep(int seconds, int milliseconds) {
#if _POSIX_C_SOURCE >= 199309L
  struct timespec t;

  t.tv_sec = seconds;
  t.tv_nsec = (long)milliseconds * 1000 * 1000;
  return nanosleep(&t, 0);
#else
  int err;
  if (seconds) {
    err = sleep(seconds);
    if (err)
      return err;
  }
  return usleep(milliseconds);
#endif
}
#endif

/** TODO: WINDOWS, MAC **/
#endif
#endif
