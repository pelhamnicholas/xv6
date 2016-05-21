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

void func(void *arg) {
  while (1) {
    printf(1, "%s\n", (char*) arg);
    sleep(10);
    thread_yield(0);
  }
}

int main(void) {

  if (thread_create(func, "thread 1") == 0)
    exit(1);

  if (thread_create(func, "thread 2") == 0)
    exit(1);

  if (thread_create(func, "thread 3") == 0)
    exit(1);

  if (thread_create(func, "thread 4") == 0)
    exit(1);

  while(wait(0) >= 0) ;

  exit(0);
}
