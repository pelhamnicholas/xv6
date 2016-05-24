#include "param.h"
#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "fcntl.h"
#include "syscall.h"
#include "traps.h"
#include "memlayout.h"

#define MAXFORKS 30

int get_option(int, char**);
int show_help(void);
int test_status(void);
int test_waitpid(void);
int test_scheduler(void);
int test_inheritance(void);

int main(int argc, char *argv[]) {
  unsigned char option = 0;

  if ((option = get_option(argc, argv)) == 0)
    show_help();

  printf(1, "\nStarting...\n", option);

  if (option & 0x10) schedinfoinit();
  if (option & 0x01) test_status();
  if (option & 0x02) test_waitpid();
  if (option & 0x04) test_scheduler();
  if (option & 0x08) test_inheritance();
  if (option & 0x10) schedinfo();

  printf(1, "\nTesting Complete\n");
  
  exit(0);
}

int get_option(int argc, char * argv[]) {
  int option = 0;
  unsigned int i;

  if (argc == 1)
    return 0x0F;

  for (i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-status") == 0) {
      option |= 0x01;
    } else if (strcmp(argv[i], "-waitpid") == 0) {
      option |= 0x02;
    } else if (strcmp(argv[i], "-scheduler") == 0) {
      option |= 0x04;
    } else if (strcmp(argv[i], "-inheritance") == 0) {
      option |= 0x08;
    } else if (strcmp(argv[i], "-performance") == 0) {
      option |= 0x10;
    }
  }

  return option;
}

int show_help(void) {
    printf(1, "Incorrect options. Valid choices are:\n");
    printf(1, "  -status          : Tests the status flag using both \n");
    printf(1, "                     exit(int) and wait(int*).\n");
    printf(1, "  -waitpid         : Tests the waitpid system call by\n");
    printf(1, "                     creating 30 processes and having\n");
    printf(1, "                     4 of them wait on a single one.\n");
    printf(1, "  -scheduler       : Tests the priority scheduler by\n");
    printf(1, "                     creating 30 processes of ascending\n");
    printf(1, "                     priority, and then 30 processes of\n");
    printf(1, "                     descending priority.\n");
    printf(1, "  -inheritance     : Tests the proprotiy inheritance using\n");
    printf(1, "                     some test system calls to use a lock\n");
    printf(1, "                     created specifically for the test.\n");
    printf(1, "  -performance     : Prints performance data for processes\n");
    printf(1, "                     running at each priority level. The \n");
    printf(1, "                     values are running averages.\n");
    exit(0);
}

int test_status(void) {
  int pid, cpid, status;
  unsigned int i;

  printf(1, "\nTesting exit(int) and wait(int*)\n\n");
  for (i = 0; i < MAXFORKS; i++) {
    pid = fork();
    if (pid == 0) { // child
      printf(1, "[%d] Exiting with status %d\n", (int) getpid(), (i % 3));
      exit(i % 3);
    } else if (pid > 0) { // parent
      cpid = (int) wait(&status);
      printf(1, "[%d] %d exited with status %d\n", 
          (int) getpid(), cpid, status);
    } else {
      printf(2, "fail\n");
      exit(-1);
    }
  }

  return 0;
}

int test_waitpid(void) {
  int pid, wpid, status, parent, cnt;
  unsigned int i;

  printf(1, "\nTesting exit(int) and waitpid(int, int*, int)\n\n");

  pid = fork();
  
  if (pid == 0) {
    parent = getpid();

    for (i = 0; i < MAXFORKS; i++) {
      pid = fork();
      if (pid == 0)
        break;
      if (pid < 0)
        printf(1, "Fail!\n");
    }
    if (pid == 0) {
      if (getpid() == parent + 3) {
        for (i = 0; i < 33554431; i++)
          cnt = (i % 2) ? cnt + i : cnt - i;
      } else if ((getpid() == parent + 7) || (getpid() == parent + 13)
              || (getpid() == parent + 17) || (getpid() == parent + 23)) {
        printf(1, "[%d]: Waiting on %d\n", getpid(), parent + 3);
        wpid = waitpid(parent + 3, &status, 0);
        printf(1, "[%d]: Done waiting on %d\n whose exit status was %d\n", 
            getpid(), parent + 3, status);
        exit(0);
      } else {
        for (i = 0; i < 1048575; i++)
          cnt = (i % 2) ? cnt + i : cnt - i;
      }
      printf(1, "[%d]: Exiting with status %d\n", getpid(), getpid() % 7);
      exit(getpid() % 7);
    } else if (pid > 0) {
      while((wpid=wait(&status)) >= 0)// && wpid != pid)
        ;
      //printf(1, "[%d]: cpid was %d\n", getpid(), cpid);
    } else {
      printf(2, "fail\n");
      exit(-1);
    }
  } else if (pid < 0) {
    printf(2, "fail\n");
    exit(-1);
  } else {
    while((wpid=wait(&status)) >= 0)// && wpid != pid)
      ;
    exit(0);
  }

  return 0;
}

int test_scheduler(void) {
  int pid, cpid, wpid, status, cnt;
  unsigned int i, n;

  printf(1, "\nTesting priority scheduler\n\n");
  setpriority(0);
  cpid = fork();
  if (cpid == 0) {
      for (n = 0; n < MAXFORKS; n++) {
        pid = fork();
        if (pid == 0) {
          setpriority(n+1);
          break;
        } else if (pid > 0) {
          setpriority(63);
        } else {
          printf(1, "fail\n");
          exit(-1);
        }
      }
      if (pid > 0) {
        printf(1, "[%d] was used to create processes of ascending priority.\n",
            (int) getpid());
        printf(1, "[%d] waiting for all children to exit.\n", (int) getpid());
        for (i = 0; i < 33554432; i++)
          cnt = (i % 2) ? cnt + i : cnt - i;
        while((wpid=wait(&status)) >= 0)// && wpid != pid)
          ;
      }
      for (i = 0; i < 8388608; i++)
        cnt = (i % 2) ? cnt + i : cnt - i;
      printf(1, "[%d] has priority: %d\n", (int) getpid(), getpriority());
      exit(0);
  } else if (cpid > 0) {
    cpid = waitpid(cpid, 0, 0);
    printf(1, "\n");
  } else {
    printf(2, "fail\n");
    exit(-1);
  }

  setpriority(0);
  cpid = fork();
  if (cpid == 0) {
      for (n = 0; n < MAXFORKS; n++) {
        pid = fork();
        if (pid == 0) {
          setpriority(MAXFORKS-n);
          break;
        } else if (pid > 0) {
          setpriority(63);
        } else {
          printf(1, "fail\n");
          exit(-1);
        }
      }
      if (pid > 0) {
        printf(1, "[%d] was used to create processes of descending priority.\n",
            (int) getpid());
        printf(1, "[%d] waiting for all children to exit.\n", (int) getpid());
        for (i = 0; i < 33554432; i++)
          cnt = (i % 2) ? cnt + i : cnt - i;
        while((wpid=wait(&status)) >= 0)// && wpid != pid)
          ;
      }
      for (i = 0; i < 8388608; i++)
        cnt = (i % 2) ? cnt + i : cnt - i;
      printf(1, "[%d] has priority: %d\n", (int) getpid(), getpriority());
      exit(0);
  } else if (cpid > 0) {
    cpid = waitpid(cpid, 0, 0);
    printf(1, "Parent [%d] has priority: %d\n\n", 
        (int) getpid(), getpriority());
  } else {
    printf(2, "fail\n");
    exit(-1);
  }

  return 0;
}

int test_inheritance(void) {
  int pid, cpid, wpid, status, cnt;
  unsigned int i;

  test_init();
  pid = fork();
  if (pid == 0) {
    setpriority(0);
    printf(1, "[%d] acquiring lock at priority %d\n", getpid(), 
        getpriority());
    update_num(134217727);
    exit(0);
  } else if (pid > 0) {
    cpid = fork();
    setpriority(63);
    if (cpid == 0) {
      setpriority(31);
      for (i = 0; i < 262144; i++)
        cnt = (i % 2) ? cnt + i : cnt - i;
      printf(1, "[%d] acquiring lock at priority %d\n", getpid(), 
          getpriority());
      update_num(134217727);
      exit(0);
    } else if (cpid > 0) {
      for (i = 0; i < 2097151; i++)
        cnt = (i % 2) ? cnt + i : cnt - i;
      printf(1, "[%d] acquiring lock at priority %d\n", getpid(), 
          getpriority());
      printf(1, "  num = %d\n", get_num());
    } else {
      printf(2, "Fail!");
      exit(-1);
    }
    while((wpid=wait(&status)) >= 0)// && wpid != pid)
      ;
    printf(1, "\n");
  } else {
    printf(2, "Fail!");
    exit(-1);
  }

  return 0;
}
