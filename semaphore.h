#include "param.h"
#include "spinlock.h"

typedef struct semaphore {
  unsigned int val;
  struct spinlock lock;
  void * thread[NPROC];
  unsigned int next;
  unsigned int end;
} semaphore;
