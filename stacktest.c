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

uint recursive_call(uint i) {
  //if ((i % 65536) == 0)
    //printf(1, "%d\n", i);
  //if (i > 7208960)
    //printf(1, "%d\n", i);
  return recursive_call(i+1);
}

int main(int argc, char * argv[]) {
  uint i;

  i = recursive_call(0);
  printf(1, "%d\n", i);

  exit(0);
}
