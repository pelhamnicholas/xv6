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
  int pid, cpid, wpid, status, cnt;
  unsigned int i, n;
  const int MAXFORKS = 10;

  printf(1, "\nStarting...\n");

  printf(1, "\nTesting exit(int) and wait(int*)\n");
  pid = fork();
  if (pid == 0) { // child
    printf(1, "Child [%d]: Exiting with status 0\n", (int) getpid());
    for (i = 0; i < 2097151; i++)
      cnt = (i % 2) ? cnt + i : cnt - i;
    exit(0);
  } else if (pid > 0) { // parent
    cpid = (int) wait(&status);
    printf(1, "Parent [%d]: cpid was %d\n", (int) getpid(), cpid);
    printf(1, "Child exit status was %d\n", status);
  } else {
    printf(2, "fail\n");
    exit(-1);
  }

  printf(1, "\nTesting exit(int) and waitpid(int, int*, int)\n");
  pid = fork();
  if (pid == 0) {
    printf(1, "Child [%d]: Exiting with status 1\n", (int) getpid());
    for (i = 0; i < 2097151; i++)
      cnt = (i % 2) ? cnt + i : cnt - i;
    exit(1);
  } else if (pid > 0) {
    cpid = (int) waitpid(pid, &status, 0);
    printf(1, "Parent [%d]: cpid was %d\n", (int) getpid(), cpid);
    printf(1, "Child exit status was %d\n", status);
  } else {
    printf(2, "fail\n");
    exit(-1);
  }
  
  printf(1, "\nTesting priority scheduler\n");
  setpriority(0);
  cpid = fork();
  if (cpid == 0) {
      for (n = 0; n < MAXFORKS; n++) {
        pid = fork();
        if (pid == 0) {
          setpriority(MAXFORKS - n);
          break;
        } else if (pid > 0) {
          setpriority(64);
        } else {
          printf(1, "fail\n");
          exit(-1);
        }
      }
      for (i = 0; i < 104857555; i++)
        cnt = (i % 2) ? cnt + i : cnt - i;
      if (pid > 0) {
        printf(1, "[%d] was used to create all other priority processes.\n", (int) getpid());
        printf(1, "[%d] waiting for all children to exit.\n", (int) getpid());
        while((wpid=wait(&status)) >= 0 && wpid != pid)
          ;
        }
      printf(1, "[%d] has priority: %d\n", (int) getpid(), getpriority());
      exit(0);
  } else if (cpid > 0) {
    for (i = 0; i < 104857555; i++)
      cnt = (i % 2) ? cnt + i : cnt - i;
    printf(1, "Parent [%d] has priority: %d\n", (int) getpid(), getpriority());
  } else {
    printf(2, "fail\n");
    exit(-1);
  }

  cpid = wait(0);
  exit(0);
}
