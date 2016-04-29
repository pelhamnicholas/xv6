#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

// New syscall that gives the number of syscalls
// added for lab 0.5
unsigned long numcalls = 0;
int sys_count(void) {
  return numcalls;
}

int
sys_setpriority(void)
{
  int priority;
  argint(0, &priority);
  setpriority(priority);
  return priority;
}

int
sys_getpriority(void)
{
    return getpriority();
}

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  int status;
  argint(0, &status);
  exit(status);
  return 0;  // not reached
}

int
sys_exitinfo(void)
{
  int status;
  argint(0, &status);
  exitinfo(status);
  return 0;  // not reached
}

int
sys_wait(void)
{
  int *status;
  argptr(0, (char **) &status, sizeof(int*));
  return wait(status);
}

int
sys_waitpid(void)
{
  int pid, options;
  int *status;
  argint(0, &pid);
  argptr(1, (char **) &status, sizeof(int*));
  argint(2, &options);
  return waitpid(pid, status, options);
}

int
sys_schedinfoinit(void)
{
  schedinfoinit();
  return 0;
}

int
sys_schedinfo(void)
{
  schedinfo();
  return 0;
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return proc->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = proc->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;
  
  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(proc->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;
  
  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

// priority inheritance testing
int
sys_test_init(void)
{
  test_init();
  return 0;
}

int
sys_update_num(void)
{
  int i;
  i = argint(0, &i);
  return update_num(i);
}

int
sys_get_num(void)
{
  return get_num();
}
