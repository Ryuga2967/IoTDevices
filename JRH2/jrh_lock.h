#ifndef __JRH_LOCK_H
#define __JRH_LOCK_H
#include <semaphore.h>

typedef struct jrh_lock jrh_lock_t;
struct jrh_lock {
  sem_t read;
  sem_t write;
  size_t n_reader;
};

typedef enum jrh_lock_type jrh_lock_type;
enum jrh_lock_type {
  JHR_LOCK_NONE = 0,
  JRH_LOCK_READ = 1,
  JRH_LOCK_WRITE = 2,
};

int jrh_lock_create(jrh_lock_t* lock);
int jrh_lock_delete(jrh_lock_t* lock);

void jrh_lock_grab(jrh_lock_t* lock, int types);
void jrh_lock_release(jrh_lock_t* lock, int types);

#endif
