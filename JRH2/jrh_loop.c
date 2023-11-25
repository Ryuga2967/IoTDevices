#include "jrh_loop.h"

static int jrh_loop_check(jrh_event_t* event) {
  if (event->type == JRH_EVENT_LOOP)
    return 0;
  return 1;
}

static int jrh_loop_work(jrh_event_t* event) {
  jrh_loop_t* loop = event->io.kev.udata;
  fprintf(stderr, "Loop changed! %ld %ld %ld\n", loop->n_ctxs, loop->n_changes, loop->n_events);
  return 0;
}

static int jrh_loop_dispatch(jrh_event_t* event) {
  event->work = jrh_loop_work;
  return 0;
}

int jrh_loop_create(jrh_loop_t* loop) {
  loop->kq = kqueue();
  loop->n_changes = 0;
  loop->changes = malloc(sizeof(kevent)*512);
  loop->n_events = 512;
  loop->events = malloc(sizeof(kevent)*512);
  loop->n_ctxs = 1;
  loop->ctxs = malloc(sizeof(jrh_event_t));
  loop->queue = NULL;
  jrh_event_assign_user(&loop->loop_event, 0, jrh_loop_check, jrh_loop_dispatch);
  loop->loop_event.prior = -1;
  loop->loop_event.io.kev.udata = loop;
  jrh_loop_register(loop, &loop->loop_event);

  return 0;
}

int jrh_loop_delete(jrh_loop_t* loop) {
  close(loop->kq);
  for (int i = 0;i < loop->n_ctxs;++i) {
    jrh_loop_unregister(loop, loop->ctxs + i);
  }
  jrh_loop_unregister(loop, &loop->loop_event);
  free(loop->changes);
  loop->n_changes = 0;
  free(loop->events);
  loop->n_events = 0;
  free(loop->ctxs);
  loop->n_ctxs = 0;
  while(loop->queue) {
    jrh_queue_pop(&loop->queue);
  }
  return 0;
}

int jrh_loop_run(jrh_loop_t* loop) {
  for (;;) {
    int neq = kevent(loop->kq, loop->changes, loop->n_changes, loop->events, loop->n_events, 0);
    loop->n_changes = 0;
    if (neq <= 0)
      continue;
      
    for (int i = 0;i < neq;++i) {
      jrh_event_t* event = NULL;
      event = loop->events[i].udata;
      if (event->type == JRH_EVENT_LOOP)
        event = &loop->loop_event;
      event->io.kev = loop->events[i];
      if (event->check(event))
        continue;
      if (event->dispatch(event))
        continue;
      if (event->work) {
        jrh_queue_push(&loop->queue, &event, sizeof(void*), event->prior);
      }
    }
    
    while (loop->queue) {
      jrh_queue_t* delete = loop->queue;
      jrh_event_t* event = *(jrh_event_t** )loop->queue->data;
      event->work(event);
      jrh_queue_pop(&loop->queue);
      free(delete);
    }
  }

  return 0;
}

struct jrh_delay_object { 
  jrh_loop_t* loop;
  jrh_event_t* orig;
  jrh_check_t check;
  jrh_dispatch_t dispatch;
  jrh_work_t work;
  void* data;
  size_t data_len;
  int time;
};

static int jrh_loop_delay_work(jrh_event_t* event) {
  struct jrh_delay_object* delay = event->io.data;
  jrh_event_t* orig = delay->orig;
  orig->check = delay->check;
  orig->io.data = delay->data;
  orig->io.data_len = delay->data_len;
  free(delay->data);
  free(event);
  return 0;
}

static int jrh_loop_delay_after_work(jrh_event_t* event) {
  struct jrh_delay_object* delay = event->io.data;
  jrh_event_t* orig = delay->orig;
  orig->work = delay->work;
  orig->io.data = delay->data;
  orig->io.data_len = delay->data_len;
  orig->work(event);

  free(delay->data);
  jrh_loop_delay(delay->loop, event, delay->time);
  return 0;
}

int jrh_loop_delay(jrh_loop_t* loop, jrh_event_t* event, int time) {
  jrh_check_t check = event->check;
  jrh_event_t* delay = malloc(sizeof(*delay));
  struct jrh_delay_object* obj = malloc(sizeof(struct jrh_delay_object));
  *obj = (struct jrh_delay_object){
    .loop = loop,
    .orig = event,
    .check = event->check,
    .dispatch = NULL,
    .work = NULL,
    .data = event->io.data,
    .data_len = event->io.data_len,
    .time = time,
  };
  event->check = NULL;
  jrh_event_assign_timer(delay, event->io.kev.ident, time, 0, jrh_loop_delay_work);
  delay->io.data = obj;
  delay->io.data_len = sizeof(*obj);
  jrh_loop_register(loop, delay);
  return 0;
}

int jrh_loop_delay_after(jrh_loop_t* loop, jrh_event_t* event, int time) {
  jrh_check_t check = event->check;
  struct jrh_delay_object* obj = malloc(sizeof(struct jrh_delay_object));
  *obj = (struct jrh_delay_object){
    .loop = loop,
    .orig = event,
    .check = NULL,
    .dispatch = NULL,
    .work = event->work,
    .data = event->io.data,
    .data_len = event->io.data_len,
    .time = time,
  };
  event->work = jrh_loop_delay_after_work;
  event->io.data = obj;
  event->io.data_len = sizeof(*obj);
  return 0;
}

int jrh_loop_register(jrh_loop_t* loop, jrh_event_t* event) {
  event->io.kev.fflags |= EV_ADD;
  loop->changes[loop->n_changes++] = event->io.kev;
  return 0;
}

int jrh_loop_unregister(jrh_loop_t* loop, jrh_event_t* event) {
  event->io.kev.fflags = EV_DELETE;
  loop->changes[loop->n_changes++] = event->io.kev;
  return 0;
}
