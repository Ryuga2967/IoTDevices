#include "jrh_queue.h"

static inline jrh_queue_t* rp_heap_merge(jrh_queue_t* root1, jrh_queue_t* root2) {
  if (!root1)
    return root2;
  if (!root2)
    return root1;

  if (root1->pri > root2->pri) {
    jrh_queue_t* tmp = root2;
    root2 = root1;
    root1 = tmp;
  }

  root2->right = root1->left;
  root1->left = root2;

  return root1;
}

static inline void rp_heap_insert(jrh_queue_t** root, void* data, size_t data_size, int pri) {
  jrh_queue_t* new = malloc(sizeof(jrh_queue_t) + data_size);
  new->pri = pri;
  new->left = NULL;
  new->right = NULL;
  memcpy(new->data, data, data_size);
  *root = rp_heap_merge(*root, new);
}

static inline void rp_heap_delete(jrh_queue_t** root) {
  if (!(*root))
    return;

  *root = (*root)->left;

  if (!(*root))
    return;

  jrh_queue_t* next = (*root)->right;
  while(next) {
    jrh_queue_t* orig_next = next->right;
    (*root)->right = NULL;
    *root = rp_heap_merge(*root, next);
    next = orig_next;
  }
}

void jrh_queue_push(jrh_queue_t** q, void* data, size_t data_size, int pri) {
  rp_heap_insert(q, data, data_size, pri);
}

jrh_queue_t* jrh_queue_pop(jrh_queue_t** q) {
  jrh_queue_t* result = *q;
  rp_heap_delete(q);
  return result;
}
