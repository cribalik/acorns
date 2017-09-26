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

WORK_QUEUE__CALL int work_queue_init(WorkQueue *queue, int num_jobs);
WORK_QUEUE__CALL int work_queue_init_ex(WorkQueue *queue, Job *job_buffer, int job_buffer_size);
WORK_QUEUE__CALL int work_queue_push(WorkQueue *queue, void (*fun)(void*), void *data);
WORK_QUEUE__CALL void work_queue_pop(WorkQueue *queue, Job *job_out);
WORK_QUEUE__CALL void work_queue_job_done(WorkQueue *queue);
WORK_QUEUE__CALL int work_queue_isdone(WorkQueue *queue, int num_jobs);

#endif /* WORK_QUEUE_H */



#ifdef WORK_QUEUE_IMPLEMENTATION

WORK_QUEUE__CALL int work_queue_init(WorkQueue *queue, int job_buffer_size) {
  Job *jobs;

  jobs = malloc(sizeof(*queue->jobs) * job_buffer_size);
  if (!jobs)
    return 1;
  return work_queue_init_ex(queue, jobs, job_buffer_size);
}

WORK_QUEUE__CALL int work_queue_init_ex(WorkQueue *queue, Job *job_buffer, int job_buffer_size) {
  /* check power of two */
  if (job_buffer_size & (job_buffer_size - 1))
    return 1;
  queue->jobs = job_buffer;
  if (!queue->jobs)
    return 1;
  queue->buf_size = job_buffer_size;
  queue->head.val = queue->tail.val = queue->num_completed.val = 0;
  semaphore_init(&queue->num_available, 0);
  return 0;
}

WORK_QUEUE__CALL int work_queue_push(WorkQueue *queue, void (*fun)(void*), void *data) {
  int head, tail, next_head;

  head = queue->head.val;
  next_head = (head+1) & (queue->buf_size-1);
  tail = atomic_read_int(&queue->tail);
  if (next_head == tail)
    return 1;

  queue->jobs[head].fun = fun;
  queue->jobs[head].data = data;
  atomic_write_int(&queue->head, next_head);
  semaphore_up(&queue->num_available);
  return 0;
}

WORK_QUEUE__CALL void work_queue_pop(WorkQueue *queue, Job *job_out) {
  int tail, next_tail;
  int size_mask;
  Job job;

  size_mask = queue->buf_size-1;

  semaphore_down(&queue->num_available);

  for (;;) {
    tail = atomic_read_int(&queue->tail);
    job = queue->jobs[tail];
    next_tail = (tail + 1) & size_mask;
    #if 1
    if (atomic_cas_int(&queue->tail, tail, next_tail))
      break;
    #else
    atomic_write_int(&queue->tail, next_tail);
    break;
    #endif
  }

  *job_out = job;
}

WORK_QUEUE__CALL void work_queue_job_done(WorkQueue *queue) {
  atomic_add_int(&queue->num_completed, 1);
}

WORK_QUEUE__CALL int work_queue_isdone(WorkQueue *queue, int num_jobs) {
  return atomic_read_int(&queue->num_completed) == num_jobs;
}

#endif