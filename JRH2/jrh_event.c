#include "jrh_event.h"
#include <sys/event.h>

static int jrh_event_fd_check(jrh_event_t* event) {
  jrh_event_type_t type = JRH_EVENT_FD;

  if (type == event->type) {
    return 0;
  }

  return 1;
}

static int jrh_event_fd_read(jrh_event_t* event) {
  size_t data_len = 64;
  event->io.data = malloc(data_len);
  int n = read(event->io.kev.ident, event->io.data, data_len);
  while (n == data_len) {
    size_t new_size = data_len * 2;
    void* new = realloc(event->io.data, new_size);
    if (!new)
      return 1;
    n = read(event->io.kev.ident, new + data_len, new_size - data_len);
    event->io.data = new;
    data_len  = new_size;
  }

  event->io.data_len = data_len;
  if (debug)
    fprintf(stderr, "%s\n", (char* )event->io.data);
  return 0;
}

static int jrh_event_fd_write(jrh_event_t* event) {
  if (debug)
    fprintf(stderr, "Write operation happened on %ld\n", event->io.kev.ident);
  return 0;
}

static int jrh_event_fd_dispatch(jrh_event_t* event) {
  int ret = 1;
  if (event->io.kev.filter == EVFILT_READ) {
    event->work = jrh_event_fd_read;
    ret = 0;
  } else if (event->io.kev.filter == EVFILT_WRITE) {
    event->work = jrh_event_fd_write;
    ret = 0;
  }

  return ret;
}

void jrh_event_assign_fd(jrh_event_t* event, int fd, int flag) {
  event->type = JRH_EVENT_FD;
  event->prior = 0;
  if (flag == 0)
    EV_SET(&event->io.kev, fd, EVFILT_READ, EV_ADD, 0, 0, event);
  else
    EV_SET(&event->io.kev, fd, EVFILT_WRITE, EV_ADD, 0, 0, event);
  event->check = jrh_event_fd_check;
  event->dispatch = jrh_event_fd_dispatch;
  event->work = NULL;
}

static int jrh_event_timer_check(jrh_event_t* event) {
  jrh_event_type_t type = JRH_EVENT_TIMER;

  if (type == event->type) {
    return 0;
  }

  return 1;
}

static int jrh_event_timer_dispatch(jrh_event_t* event) {
  int ret = 1;
  if (event->io.kev.filter == EVFILT_TIMER) {
    ret = 0;
  }

  return ret;
}

void jrh_event_assign_timer(jrh_event_t* event, int id, int time, int repeat, jrh_work_t work) {
  event->type = JRH_EVENT_TIMER;
  event->prior = 0;
  if (repeat)
    EV_SET(&event->io.kev, id, EVFILT_TIMER, EV_ADD, NOTE_USECONDS, time, event);
  else
    EV_SET(&event->io.kev, id, EVFILT_TIMER, EV_ONESHOT, NOTE_USECONDS, time, event);
  event->check = jrh_event_timer_check;
  event->dispatch = jrh_event_timer_dispatch;
  event->work = work;
}

void jrh_event_assign_user(jrh_event_t* event, int id, jrh_check_t check, jrh_dispatch_t dispatch) {
  event->type = JRH_EVENT_USER;
  event->prior = 0;
  EV_SET(&event->io.kev, id, EVFILT_USER, EV_ADD, 0, 0, event);
  event->check = check;
  event->dispatch = dispatch;
  event->work = NULL;
}
