#ifndef __JRH_EVENT_H
#define __JRH_EVENT_H
#include <sys/event.h>
#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int debug = 0;

typedef enum jrh_event_type jrh_event_type_t;
enum jrh_event_type {
  JRH_EVENT_LOOP,
  JRH_EVENT_FD,
  JRH_EVENT_TIMER,
  JRH_EVENT_USER,
};

typedef struct jrh_event_io jrh_event_io_t;
struct jrh_event_io {
  struct kevent kev;
  void* data;
  size_t data_len;
};

typedef struct jrh_event jrh_event_t;
struct jrh_event {
  jrh_event_type_t type;
  jrh_event_io_t io;
  int prior;
  int (* check)(jrh_event_t* );
  int (* dispatch)(jrh_event_t* );
  int (* work)(jrh_event_t* );
};

typedef int (* jrh_check_t)(jrh_event_t* );
typedef int (* jrh_dispatch_t)(jrh_event_t* );
typedef int (* jrh_work_t)(jrh_event_t* );

void jrh_event_assign_fd(jrh_event_t* event, int fd, int flag);
void jrh_event_assign_timer(jrh_event_t* event, int id, int time, int repeat, jrh_work_t work);
void jrh_event_assign_user(jrh_event_t* event, int id, jrh_check_t check, jrh_dispatch_t dispatch);

#endif
