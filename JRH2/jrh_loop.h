#ifndef __JRH_LOOP_H
#define __JRH_LOOP_H
#include "jrh_event.h"
#include "jrh_queue.h"
#include "jrh_lock.h"

typedef struct jrh_loop jrh_loop_t;
struct jrh_loop {
  int kq;
  struct kevent* changes;
  size_t n_changes;
  struct kevent* events;
  size_t n_events;
  jrh_event_t* ctxs;
  size_t n_ctxs;
  jrh_event_t loop_event;
  jrh_queue_t* queue;
};

int jrh_loop_run(jrh_loop_t* loop);

int jrh_loop_create(jrh_loop_t* loop);
int jrh_loop_delete(jrh_loop_t* loop);

int jrh_loop_delay(jrh_loop_t* loop, jrh_event_t* event, int time);
int jrh_loop_delay_after(jrh_loop_t* loop, jrh_event_t* event, int time);

int jrh_loop_register(jrh_loop_t* loop, jrh_event_t* event); 
int jrh_loop_unregister(jrh_loop_t* loop, jrh_event_t* event); 

#endif
