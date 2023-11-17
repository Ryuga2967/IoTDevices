#include "jrh_loop.h"
#include <stdlib.h>
#include <unistd.h>

jrh_loop_t pool = { 0 };

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
  fprintf(stderr, "%s\n", (char* )event->io.data);
  free(event->io.data);
  return 0;
}

static int jrh_event_fd_dispatch(jrh_event_t* event) {
  int ret = 1;
  if (event->io.kev.filter == EVFILT_READ) {
    event->work = jrh_event_fd_read;
    ret = 0;
  }

  return ret;
}

int main(void) {
  jrh_loop_create(&pool);
  jrh_event_t event_fd;
  jrh_event_assign_fd(&event_fd, STDIN_FILENO, 0);
  event_fd.dispatch = jrh_event_fd_dispatch;
  jrh_loop_register(&pool, &event_fd);
 
  jrh_loop_run(&pool);
  jrh_loop_delete(&pool);

  return 0;
}
