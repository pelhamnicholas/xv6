#include "param.h"
#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "fcntl.h"
#include "syscall.h"
#include "traps.h"
#include "memlayout.h"
#include "semaphore.h"

#define NUMTHREADS 20

semaphore frisbee_sem;
int thrower;

void player(void *arg_ptr);

int main(int argc, char *argv[]) {
  int i;

  sem_init(&frisbee_sem, 1);

  for (i = 0; i < NUMTHREADS; i++) {
    thread_create(player, (void*)&i);
    sleep(10);
  }
  while(wait(0)>=0) ;
  exit(0);
}

void player(void *arg_ptr) {
  int i, self;
  int *num = (int*) arg_ptr;
  self = *num;

  for (i = 0; i < 10; i++) {
    if (thrower != self) {
      sem_wait(&frisbee_sem);
      printf(1, "%d caught frisbee from %d\n", self, thrower);
      thrower = self;
      printf(1, "  throwing frisbee\n", self);
      sleep(20);
      sem_signal(&frisbee_sem);
    }
    sleep(20);
  }
  exit(0);
}
