#include "types.h"
//#include "defs.h"
//#include "stat.h"
#include "user.h"
#include "param.h"
//#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
//#include "spinlock.h"


int thread_create(void(*start_routine)(void *), void *arg) {
  void * stack, * mem;
  int pid;

  // Set up new threads stack
  mem = malloc(2*PGSIZE);
  if ((uint)mem % PGSIZE)
    stack = mem + (PGSIZE - ((uint)mem % PGSIZE));
  else
    stack = mem;

  if ((pid = clone((void *)stack, PGSIZE)) != 0)
    return pid;

  if (pid == 0) {
    start_routine(arg);
    free(mem);
    thread_exit(0);
  }

  return pid;
}

int thread_yield(void * stack) {
  return 0;
}

int thread_join(int tid, int *status) {
  return 0;
}
