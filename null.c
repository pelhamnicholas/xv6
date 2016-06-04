#include "param.h"
#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "fcntl.h"
#include "syscall.h"
#include "traps.h"
#include "memlayout.h"

int main(int argc, char *argv[]) {
  int *p;
  p = 0;
  printf(1, "%d: p = %x\n", getpid(), *p);
  exit(0);
}
