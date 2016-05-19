#include "types.h"
#include "user.h"
#include "param.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"

int thread_create(void(*start_routine)(void *), void *arg) {
  void * stack, * mem;
  int pid;

  // Set up new threads stack
  mem = malloc(2*PGSIZE);
  if ((uint)mem % PGSIZE)
    stack = mem + (PGSIZE - ((uint)mem % PGSIZE));
  else
    stack = mem;

  if ((pid = clone((void *)stack)) != 0)
    return pid;

  start_routine(arg);
  free(mem);
  thread_exit(0);

  return pid;
}

// does what waitpid does for now
int thread_join(int tid, int *status) {
  return waitpid(tid, status, 0);
}
