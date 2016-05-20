#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "semaphore.h"

void sem_init(struct semaphore * s, uint i) {
  int j;
  s->val = i;
  //cprintf("%s sem_init: val = %d\n", name, s->val);
  initlock(&s->lock, (char*)s);
  for (j = 0; j < NPROC; j++)
    s->thread[j] = 0;
  s->next = s->end = 0;
}

void sem_wait(struct semaphore * s) {
  acquire(&s->lock);
  //s->thread[s->end] = proc;
  //s->end = (s->end + 1) % NPROC;
  while (s->val == 0) {
    s->thread[s->end] = proc;
    s->end = (s->end + 1) % NPROC;
    sleep(proc, &s->lock);
  }
  s->val = s->val - 1;
  //cprintf("%s sem_wait: val = %d\n", s->name, s->val);
  release(&s->lock);
}

void sem_signal(struct semaphore * s) {
  acquire(&s->lock);
  s->val = s->val + 1;
  //cprintf("%s sem_signal: val = %d\n", s->name, s->val);
  if (s->thread[s->next]) {
    wakeup(s->thread[s->next]);
    s->thread[s->next] = 0;
    s->next = (s->next + 1) % NPROC;
  }
  release(&s->lock);
}

void sem_broadcast(struct semaphore * s) {
  int i;

  acquire(&s->lock);
  s->val = s->val + 1;
  //cprintf("%s sem_signal: val = %d\n", s->name, s->val);
  s->next = (s->next + 1) % NPROC;
  for (i = 0; i < NPROC; i++)
    wakeup(s->thread[i]);
  release(&s->lock);
}
