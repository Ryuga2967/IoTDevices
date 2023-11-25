#include "jrh_lock.h"

int jrh_lock_create(jrh_lock_t* lock) {
  if (-1 == sem_init(&lock->read, 0, 1))
    return JRH_LOCK_READ;
  if (-1 == sem_init(&lock->write, 0, 1))
    return JRH_LOCK_WRITE;
  lock->n_reader = 0;
  return JHR_LOCK_NONE;
}

int jrh_lock_delete(jrh_lock_t* lock) {
  if (-1 == sem_destroy(&lock->read))
    return JRH_LOCK_READ;
  if (-1 == sem_destroy(&lock->write))
    return JRH_LOCK_WRITE;
  return JHR_LOCK_NONE;
}

void jrh_lock_grab(jrh_lock_t* lock, int types) {
  if (types & JRH_LOCK_READ) {
    sem_wait(&lock->read);
    if (lock->n_reader == 0)
      sem_wait(&lock->write);
    ++lock->n_reader;
    sem_post(&lock->read);
  }
  if (types & JRH_LOCK_WRITE)
    sem_wait(&lock->write);
}

void jrh_lock_release(jrh_lock_t* lock, int types) {
  if (types & JRH_LOCK_READ) {
    sem_wait(&lock->read);
    --lock->n_reader;
    if (lock->n_reader == 0) {
      sem_post(&lock->write);
    } else if (lock->n_reader < 0) {
      lock->n_reader = 0;
    }
    sem_post(&lock->read);
  }
  if (types & JRH_LOCK_WRITE)
    sem_post(&lock->write);
}
