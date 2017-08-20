/** HEADER **/
#ifndef THREAD_H
#define THREAD_H

#if !defined(__linux__) && !defined(__APPLE__) && !defined(_MSC_VER)
#error "Unknown platform"
#endif



/** LINUX HEADER **/
#if defined(__linux__)

#ifndef __GNUC__
#error "Only gcc compiler supported for linux"
#endif

#include <pthread.h>
typedef struct {pthread_t pthread; void (*proc)(void*); void *arg;} Thread;
typedef pthread_mutex_t Thread_Mutex;

#if _XOPEN_SOURCE >= 500 || _POSIX_C_SOURCE >= 199309L
#define THREAD__HAS_SLEEP_MILLI
#endif




/** WINDOWS HEADER **/
#elif defined(_MSC_VER)

#include <windows.h>

typedef struct {HANDLE handle; void (*proc)(void*); void *arg;} Thread;
typedef HANDLE Thread_Mutex;
#define THREAD__HAS_SLEEP_MILLI



/** TODO: MAC HEADER **/
#else
#error Unimplemented platform
#endif



/** GENERIC HEADER **/

int thread_create(Thread *thread, void (*proc)(void*), void *arg);
int thread_free(Thread *thread);
int thread_join(Thread thread);
int thread_mutex_init(Thread_Mutex *mutex);
long thread_cas(long *ptr, long expected, long newval);
long thread_add(long *ptr, long n);
void thread_barrier(void);
int thread_sleep(int seconds);
#ifdef THREAD__HAS_SLEEP_MILLI
int thread_sleep_milli(int seconds, int milliseconds);
#endif

#endif

/** IMPLEMENTATION **/
#ifdef THREAD_IMPLEMENTATION



/** LINUX IMPLEMENTATION **/
#if defined(__linux__)

#include <stdlib.h>
#include <unistd.h>
#include <time.h>

void *thread__pthread_proc_wrapper(void *arg) {
  Thread *thread = arg;
  thread->proc(thread->arg);
  return 0;
}

int thread_create(Thread *thread, void (*proc)(void*), void *arg) {
  thread->arg = arg;
  thread->proc = proc;
  return pthread_create(&thread->pthread, 0, thread__pthread_proc_wrapper, thread);
}

#define thread_free(thread) 0

int thread_join(Thread thread) {
  return pthread_join(thread.pthread, 0);
}

#define thread_mutex_init(mutex_ptr) pthread_mutex_init((mutex_ptr), 0)
#define thread_lock(mutex_ptr)       pthread_mutex_lock(mutex_ptr)
#define thread_unlock(mutex_ptr)     pthread_mutex_unlock(mutex_ptr)
#define thread_cas(ptr, expected, new_val) __sync_val_compare_and_swap(ptr, expected, new_val)
#define thread_add(ptr, n)                     __sync_fetch_and_add(ptr, n)
#define thread_barrier()                       __sync_synchronize()
#define thread_sleep(seconds) sleep(seconds)

#ifdef THREAD__HAS_SLEEP_MILLI
int thread_sleep_milli(int seconds, int milliseconds) {
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



/** WINDOWS IMPLEMENTATION **/
#elif defined(_MSC_VER)

DWORD WINAPI thread__winthread_proc_wrapper(LPVOID arg) {
  Thread *thread = (Thread*)arg;
  thread->proc(thread->arg);
  return 0;
}

int thread_create(Thread *thread, void (*proc)(void*), void *arg) {
  thread->arg = arg;
  thread->proc = proc;
  thread->handle = CreateThread(0, 0, thread__winthread_proc_wrapper, thread, 0, 0);
  return thread->handle == 0;
}

#define thread_free(thread) (!CloseHandle(thread.handle))
#define thread_join(thread) WaitForSingleObject(thread.handle, INFINITE)
#define thread_mutex_init(mutex_ptr) WaitForSingleObject(*(mutex_ptr), INFINITE)
#define thread_cas(ptr, expected, new_val) InterlockedCompareExchange((LONG volatile*)(ptr), (new_val), (expected))
#define thread_add(ptr, n) InterlockedExchangeAdd((LONG volatile*)(ptr), (n))
#define thread_barrier() (_ReadWriteBarrier, MemoryBarrier())
#define thread_sleep(seconds) Sleep((seconds) * 1000)
#define thread_sleep_milli(seconds, milliseconds) Sleep((seconds * 1000) + (milliseconds))






/** TODO: MAC IMPLEMENTATION **/
#else
#error Unimplemented platform
#endif
#endif
