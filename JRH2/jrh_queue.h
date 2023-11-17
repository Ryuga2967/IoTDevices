#ifndef __JRH_QUEUE_H
#define __JRH_QUEUE_H

#include <stdlib.h>
#include <string.h>

typedef struct jrh_queue jrh_queue_t;
struct jrh_queue {
  int pri;
  struct jrh_queue* left;
  struct jrh_queue* right;
  unsigned char data[];
};

void jrh_queue_push(jrh_queue_t** q, void* data, size_t data_size, int pri);
jrh_queue_t* jrh_queue_pop(jrh_queue_t** q);

#endif
