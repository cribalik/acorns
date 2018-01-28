#ifndef WORK_QUEUE_H
#define WORK_QUEUE_H

#include "thread.h"
#include <stdlib.h>

#ifndef WORK_QUEUE_NO_STATIC
  #define WORK_QUEUE__CALL static
#else
  #define WORK_QUEUE__CALL
#endif

typedef struct Job Job;
typedef struct WorkQueue WorkQueue;

struct Job {
  void (*fun)(void*);
  void *data;
};

struct WorkQueue {
  Job *jobs;
  int buf_size;
  Semaphore num_available;
  AtomicInt head, tail, num_completed;
};

typedef enum {
  WORK_QUEUE_ERROR       = 1,
  WORK_QUEUE_MALLOC_FAIL = 2,
  WORK_QUEUE_SEM_FAIL    = 3,
  WORK_QUEUE_INV_ARG     = 4,
  WORK_QUEUE_FULL        = 5
} WQ_ERR;

WORK_QUEUE__CALL WQ_ERR work_queue_init(WorkQueue *queue, Job *job_buffer, int job_buffer_size);
WORK_QUEUE__CALL WQ_ERR work_queue_push(WorkQueue *queue, void (*fun)(void*), void *data);
WORK_QUEUE__CALL WQ_ERR work_queue_pop(WorkQueue *queue, Job *job_out);
WORK_QUEUE__CALL void work_queue_job_done(WorkQueue *queue);
WORK_QUEUE__CALL WQ_ERR work_queue_isdone(WorkQueue *queue, int num_jobs);
WORK_QUEUE__CALL WQ_ERR work_queue_free(WorkQueue *queue);

#endif /* WORK_QUEUE_H */



#ifdef WORK_QUEUE_IMPLEMENTATION

WORK_QUEUE__CALL WQ_ERR work_queue_init(WorkQueue *queue, Job *job_buffer, int job_buffer_size) {
  /* check power of two */
  if (job_buffer_size & (job_buffer_size - 1))
    return WORK_QUEUE_INV_ARG;
  queue->jobs = job_buffer;
  if (!queue->jobs)
    return WORK_QUEUE_INV_ARG;
  queue->buf_size = job_buffer_size;
  queue->head.val = queue->tail.val = queue->num_completed.val = 0;
  if (semaphore_init(&queue->num_available, 0, job_buffer_size+1))
    return WORK_QUEUE_SEM_FAIL;
  return 0;
}

WORK_QUEUE__CALL WQ_ERR work_queue_push(WorkQueue *queue, void (*fun)(void*), void *data) {
  int head, tail, next_head;

  // head will only be accessed by producer (which is single threaded)
  head = queue->head.val;
  next_head = (head+1) & (queue->buf_size-1);
  tail = atomic_read_int(&queue->tail);
  if (next_head == tail)
    return WORK_QUEUE_FULL;

  queue->jobs[head].fun = fun;
  queue->jobs[head].data = data;
  atomic_write_int(&queue->head, next_head);
  if (semaphore_up(&queue->num_available))
    return WORK_QUEUE_SEM_FAIL;
  return 0;
}

WORK_QUEUE__CALL WQ_ERR work_queue_pop(WorkQueue *queue, Job *job_out) {
  int tail, next_tail;
  int size_mask;
  Job job = {0};

  size_mask = queue->buf_size-1;

  if (semaphore_down(&queue->num_available))
    return WORK_QUEUE_SEM_FAIL;

  for (;;) {
    tail = atomic_read_int(&queue->tail);
    job = queue->jobs[tail];
    next_tail = (tail + 1) & size_mask;
    #if 1
    if (atomic_cas_int(&queue->tail, tail, next_tail) == tail)
      break;
    #else
    atomic_write_int(&queue->tail, next_tail);
    break;
    #endif
  }

  *job_out = job;
  return 0;
}

WORK_QUEUE__CALL void work_queue_job_done(WorkQueue *queue) {
  atomic_add_int(&queue->num_completed, 1);
}

WORK_QUEUE__CALL WQ_ERR work_queue_isdone(WorkQueue *queue, int num_jobs) {
  return atomic_read_int(&queue->num_completed) == num_jobs;
}

WORK_QUEUE__CALL WQ_ERR work_queue_free(WorkQueue *queue) {
  if (semaphore_free(&queue->num_available))
    return WORK_QUEUE_SEM_FAIL;
  return 0;
}

#endif