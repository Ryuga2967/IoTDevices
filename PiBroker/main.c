#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <dlfcn.h>
#include "event.h"

static broker_event_loop_t loop;
static broker_message_queue_t queue;

int handler(broker_event_t* event, short revents) {
  if (POLLIN & revents) {
    int ret = read(event->fd.sock, event->data, event->data_len - 1);
    if (ret <= 0)
      return -1;
    
    ((char* )event->data)[ret - 1] = 0;
    return 0;
  }

  return 1;
}

int callback(void* data, uint32_t data_len) {
  ((char* )data)[data_len] = 0;
  printf("cmdline: %s\n", (char* )data);

  return 0;
}

int main(int argc, char** args) {
  loop.timeout = 1000*1000;
  loop.fds = malloc(sizeof(struct pollfd)*10);
  loop.events = malloc(sizeof(broker_event_t)*10);
  loop.event_len = 0;
  loop.event_cap = 10;

  queue.head = 0;
  queue.rear = 0;
  queue.empty = 1;
  queue.messages = malloc(sizeof(broker_message_t)*64);
  queue.messages_len = 64;

  broker_event_loop_add_event(&loop, fileno(stdin), handler, callback, malloc(1024), 1024);
  
  for (int i = 1;i < argc;++i) {
    void* plugin = dlopen(args[i], 0);
    if (plugin == NULL) {
      printf("No such plugin\n");
      continue;
    }
    int (* pluginInit)(broker_event_loop_t* loop) = dlsym(plugin, "PluginInit");
    pluginInit(&loop);
  }

  while (1) {
    broker_event_loop_main(&loop, &queue);
    
    broker_message_t message;
    while (broker_message_queue_consume(&queue, &message) == 0) {
      fprintf(stderr, "%s\n", message.topic);
      if (message.callback) {
        message.callback(message.data, message.data_len);
      }
    }
    usleep(1);
  }

  return 0;
}
