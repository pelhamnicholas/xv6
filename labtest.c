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
  int pid, cpid, status;

  printf(1, "\nStarting...\n");

  printf(1, "\nTesting exit(int) and wait(int*)\n");
  pid = fork();
  if (pid == 0) { // child
    printf(1, "Child: %d\n", (int) getpid());
    exit(0);
  } else if (pid > 0) { // parent
    cpid = (int) wait(&status);
    printf(1, "Parent %d: cpid was %d\n", (int) getpid(), cpid);
    printf(1, "Child exit status was %d\n", status);
  } else {
    printf(2, "fail\n");
    exit(-1);
  }

  printf(1, "\nTesting exit(int) and waitpid(int, int*, int)\n");
  pid = fork();
  if (pid == 0) { // child
    printf(1, "Child %d: Exiting\n", (int) getpid());
    exit(0);
  } else if (pid > 0) { // parent
    cpid = (int) waitpid(pid, &status, 0);
    printf(1, "Parent %d: cpid was %d\n", (int) getpid(), cpid);
    printf(1, "Child exit status was %d\n", status);
  } else {
    printf(2, "fail\n");
    exit(-1);
  }

  printf(1, "\nFinished\n");
  exit(0);
}
