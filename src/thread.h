/** HEADER **/
#ifndef THREAD_H
#define THREAD_H


/* define to disable static */
#ifndef THREAD_NO_STATIC
  #define THREAD__CALL static
#else
  #define THREAD__CALL
#endif /* THREAD_NO_STATIC */


/** LINUX HEADER **/
#if defined(__linux__)

  #ifndef __GNUC__
  #error "Only gcc compiler supported for linux"
  #endif

  #include <pthread.h>
  #include <semaphore.h>
  typedef struct {
    pthread_t pthread;
    void (*proc)(void*);
    void *arg;
  } Thread;
  typedef sem_t Semaphore;

  #if _XOPEN_SOURCE >= 500 || _POSIX_C_SOURCE >= 199309L
    #define THREAD__HAS_SLEEP_MILLI
  #endif

/** WINDOWS HEADER **/
#elif defined(_MSC_VER)

  #include <windows.h>

  typedef struct {
    HANDLE handle;
    void (*proc)(void*);
    void *arg;
  } Thread;
  typedef HANDLE Semaphore;
  #define THREAD__HAS_SLEEP_MILLI

/** TODO: MAC HEADER **/
#else
  #error Unimplemented platform
#endif /* PLATFORM */



/** GENERIC HEADER **/

/* Embeded in struct to prevent type conversion */
typedef struct {
  int val;
} AtomicInt;

typedef struct {
  void *val;
} AtomicPtr;

/* thread */

THREAD__CALL int thread_create(Thread *thread, void (*proc)(void*), void *arg);
THREAD__CALL int thread_free(Thread *thread);
THREAD__CALL int thread_join(Thread *thread);

/* semaphore */

THREAD__CALL int semaphore_init(Semaphore *sem, int value);
THREAD__CALL int semaphore_up(Semaphore *sem);
THREAD__CALL int semaphore_down(Semaphore *sem);

/* spinlock */

typedef struct {
  AtomicInt taken;
} SpinLock;

static void spinlock_init(SpinLock *lock);
static void spinlock_lock(SpinLock *lock);
static void spinlock_unlock(SpinLock *lock);


/* atomics 
 *
 * all functions imply memory and compiler barriers
 */
THREAD__CALL int atomic_read_int(AtomicInt *ptr);
THREAD__CALL void atomic_write_int(AtomicInt *ptr, int val);
/* @return: 1 if success, 0 otherwise */
THREAD__CALL int atomic_cas_int(AtomicInt *ptr, int expected, int newval);
/* @return: old value */
THREAD__CALL int atomic_add_int(AtomicInt *ptr, int n);
THREAD__CALL int atomic_add_ptr(AtomicPtr *ptr, int n);

/* sleep */
THREAD__CALL int thread_sleep(int seconds);
#ifdef THREAD__HAS_SLEEP_MILLI
THREAD__CALL int thread_sleep_millis(int milliseconds);
#endif /* THREAD__HAS_SLEEP_MILLI */




/* HEADER IMPLEMENTATION */

#if defined(__linux__)
#define atomic_barrier() __sync_synchronize()
#elif defined(_MSC_VER)
#define atomic_barrier() (_ReadWriteBarrier)
#endif /* PLATFORM */




#endif /* THREAD_H */





/** IMPLEMENTATION **/
#ifdef THREAD_IMPLEMENTATION



/** GENERIC IMPLEMENTATION **/

/* for x86, amd64 and ARM, stores and reads don't tear ...i think */
#if !defined(__i386) && !defined(_M_IX86) && !defined(__amd64__) && !defined(_M_X64) && !defined(__arm__) && !defined(_M_ARM)
  #error "Atomics only implemented for ARM, x86 and amd64"
#endif
THREAD__CALL int atomic_read_int(AtomicInt *a) {
  int v;

  atomic_barrier();
  v = a->val;
  atomic_barrier();

  return v;
}

THREAD__CALL void atomic_write_int(AtomicInt *ptr, int x) {
  atomic_barrier();
  ptr->val = x;
  atomic_barrier();
}

THREAD__CALL void *atomic_read_ptr(AtomicPtr *a) {
  void *v;

  atomic_barrier();
  v = a->val;
  atomic_barrier();

  return v;
}

THREAD__CALL void atomic_write_ptr(AtomicPtr *ptr, void *x) {
  atomic_barrier();
  ptr->val = x;
  atomic_barrier();
}

THREAD__CALL void spinlock_init(SpinLock *lock) {
  lock->taken.val = 0;
}

THREAD__CALL void spinlock_lock(SpinLock *lock) {
  for (;;) {
    if (atomic_cas_int(&lock->taken, 0, 1))
      return;
  }
}

static void spinlock_unlock(SpinLock *lock) {
  atomic_write_int(&lock->taken, 0);
}



/** LINUX IMPLEMENTATION **/
#if defined(__linux__)

#include <stdlib.h>
#include <unistd.h>
#include <time.h>

THREAD__CALL void *thread__pthread_proc_wrapper(void *arg) {
  Thread *thread = arg;
  thread->proc(thread->arg);
  return 0;
}

THREAD__CALL int thread_create(Thread *thread, void (*proc)(void*), void *arg) {
  thread->arg = arg;
  thread->proc = proc;
  return pthread_create(&thread->pthread, 0, thread__pthread_proc_wrapper, thread);
}

THREAD__CALL int thread_free(Thread *thread) {
  (void)thread;
  return 0;
}

THREAD__CALL int thread_join(Thread *thread) {
  return pthread_join((thread)->pthread, 0);
}

THREAD__CALL int thread_sleep(int seconds) {
  return sleep(seconds);
}

THREAD__CALL int semaphore_init(Semaphore *sem, int val) {
  return sem_init(sem, 0, val);
}

THREAD__CALL int semaphore_up(Semaphore *sem) {
  return sem_post(sem);
}

THREAD__CALL int semaphore_down(Semaphore *sem) {
  return sem_wait(sem);
}

#ifdef THREAD__HAS_SLEEP_MILLI
THREAD__CALL int thread_sleep_millis(int milliseconds) {
#if _POSIX_C_SOURCE >= 199309L
  struct timespec t;

  t.tv_sec = milliseconds/1000;
  t.tv_nsec = (long)(milliseconds%1000) * 1000 * 1000;
  return nanosleep(&t, 0);
#else
  int err, seconds;
  seconds = milliseconds / 1000;
  if (seconds) {
    err = sleep(seconds);
    if (err)
      return err;
  }
  return usleep(milliseconds);
#endif
}
#endif

THREAD__CALL int atomic_cas_int(AtomicInt *ptr, int expected, int new_val) {
  return __sync_bool_compare_and_swap(&ptr->val, expected, new_val);
}

THREAD__CALL int atomic_cas_ptr(AtomicPtr *ptr, void *expected, void *new_val) {
  return __sync_bool_compare_and_swap(&ptr->val, expected, new_val);
}

THREAD__CALL int atomic_add_int(AtomicInt *ptr, int n) {
  return __sync_fetch_and_add(&ptr->val, n);
}


/** WINDOWS IMPLEMENTATION **/
#elif defined(_MSC_VER)

THREAD__CALL DWORD WINAPI thread__winthread_proc_wrapper(LPVOID arg) {
  Thread *thread = (Thread*)arg;
  thread->proc(thread->arg);
  return 0;
}

THREAD__CALL int thread_create(Thread *thread, void (*proc)(void*), void *arg) {
  thread->arg = arg;
  thread->proc = proc;
  thread->handle = CreateThread(0, 0, thread__winthread_proc_wrapper, thread, 0, 0);
  return thread->handle == 0;
}

THREAD__CALL int thread_free(Thread *thread) {
  return !CloseHandle(thread->handle);
}

THREAD__CALL int thread_join(Thread *thread) {
  return WaitForSingleObject(thread->handle, INFINITE);
}

THREAD__CALL int thread_sleep(int seconds) {
  return Sleep((seconds) * 1000);
}

THREAD__CALL int thread_sleep_millis(int milliseconds) {
  return Sleep(milliseconds);
}

THREAD__CALL int semaphore_init(Semaphore *sem, int val) {
  return CreateSemaphore(0, val, INFINITE, 0);
}

THREAD__CALL int semaphore_down(Semaphore *sem, int val) {
  return WaitForSingleObject(*sem, INFINITE);
}

THREAD__CALL int semaphore_up(Semaphore *sem) {
  return (int)ReleaseSemaphore(sem, 1, 0);
}

THREAD__CALL int atomic_cas_int(Atomic *ptr, int expected, int new_val) {
  return InterlockedCompareExchange((long*)&ptr->val, (long)new_val, (long)expected) == expected;
}

THREAD__CALL int atomic_cas_ptr(void **ptr, void *expected, void *newval) {
  return InterlockedCompareExchangePointer(ptr, new_val, expected) == expected;
}

THREAD__CALL int atomic_add_int(Atomic *ptr, int n) {
  return __sync_fetch_and_add(&ptr->val, n);
}

#define atomic_cas_int(ptr, expected, new_val) 
#define atomic_add(ptr, n) InterlockedExchangeAdd((LONG volatile*)(&(ptr)->val), (n))


/** TODO: MAC IMPLEMENTATION **/
#else
#error Unimplemented platform
#endif /* PLATFORM */

#endif /* THREAD_IMPLEMENTATION */
