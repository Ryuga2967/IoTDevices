#ifndef __BROKER_EVENT_H
#define __BROKER_EVENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/poll.h>

typedef struct broker_event_fd broker_event_fd_t;
typedef struct broker_event broker_event_t;
typedef struct broker_message broker_message_t;
typedef struct broker_event_loop broker_event_loop_t;
typedef struct broker_message_queue broker_message_queue_t;

typedef int (* broker_event_cb)(void* data, uint32_t data_len);
typedef int (* broker_event_handler)(broker_event_t* event, short revents);

struct broker_event_fd {
  int sock;
  FILE* readfp;
  FILE* writefp;
};

struct broker_event {
  broker_event_fd_t fd;
  broker_event_cb callback;
  broker_event_handler handler;
  void* data;
  uint32_t data_len;
};

struct broker_message {
  char topic[32];
  broker_event_cb callback;
  void* data;
  uint32_t data_len;
};

struct broker_listner {
  char topic[32];
  int (* callback)(void* data, uint32_t data_len);
};

struct broker_event_loop {
  int timeout;
  struct pollfd* fds;
  broker_event_t* events;
  uint32_t event_len;
  uint32_t event_cap;
};

struct broker_message_queue {
  int head;
  int rear;
  int empty;
  broker_message_t* messages;
  uint32_t messages_len;
};

int broker_message_queue_produce(
    broker_message_queue_t* queue,
    broker_message_t* message
    )
{
  if (queue->head == queue->rear && !queue->empty) {
    return 1;
  }

  queue->messages[queue->head] = *message;
  queue->head = (queue->head + 1)%queue->messages_len;
  queue->empty = 0;
  
  return 0;
}

int broker_message_queue_consume(
    broker_message_queue_t* queue,
    broker_message_t* message
    )
{
  if (queue->rear == queue->head) {
    return 1;
  }

  *message = queue->messages[queue->rear];
  queue->rear = (queue->rear + 1)%queue->messages_len;
  if (queue->rear == queue->head)
    queue->empty = 1;
  
  return 0;
}

int broker_event_loop_add_event(
    broker_event_loop_t* loop,
    int fd,
    broker_event_handler handler,
    broker_event_cb callback,
    void* data,
    uint32_t data_len)
{
  while (loop->event_cap < loop->event_len + 1) {
    loop->event_cap *= 2;
    loop->events = (broker_event_t* )realloc(loop->events, sizeof(broker_event_t)*loop->event_cap);
    loop->fds = (struct pollfd* )realloc(loop->fds, sizeof(struct pollfd)*loop->event_cap);

    if (loop->events == NULL)
      return 1;

    if (loop->fds == NULL)
      return 1;
  }

  broker_event_t* new_event = loop->events + loop->event_len;
  struct pollfd* new_fd = loop->fds + loop->event_len;

  new_event->fd.sock = fd;
  new_event->fd.readfp = fdopen(fd, "r");
  new_event->fd.writefp = fdopen(fd, "w");
  new_event->handler = handler;
  new_event->callback = callback;
  new_event->data = data;
  new_event->data_len = data_len;
  
  new_fd->fd = fd;
  new_fd->events = 0x00FF;
  new_fd->revents = 0;


  loop->event_len += 1;
  return 0;
}

int broker_event_loop_main(broker_event_loop_t* loop, broker_message_queue_t* queue) {
  if (loop->events == NULL)
    return 1;
  int retval = poll(loop->fds, loop->event_len, loop->timeout);
  
  if (retval == -1) {
    perror("pselect");
  }
  else if (retval) {
    for (int i = 0;i < loop->event_len;++i) {
      if (loop->fds[i].revents == 0)
        continue;
      int ret = loop->events[i].handler(loop->events, loop->fds[i].revents);
      if (ret < 0)
        perror("Something Error");
      else if (ret == 0) {
        broker_message_t message;
        snprintf(message.topic, 32, "%d produce", loop->events[i].fd.sock);
        message.data = loop->events[i].data;
        message.data_len = loop->events[i].data_len;
        message.callback = loop->events[i].callback;
        broker_message_queue_produce(queue, &message); 
      }

      loop->fds[i].revents = 0;
    }
  } else {
    printf("Timeout\n");
  }
  
  return 0;
}

#endif
