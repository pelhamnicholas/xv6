#include "syscall.h"
#include "traps.h"

#define SYSCALL(name) \
  .globl name; \
  name: \
    movl $SYS_ ## name, %eax; \
    int $T_SYSCALL; \
    ret

SYSCALL(fork)
SYSCALL(exit)
SYSCALL(wait)
SYSCALL(pipe)
SYSCALL(read)
SYSCALL(write)
SYSCALL(close)
SYSCALL(kill)
SYSCALL(exec)
SYSCALL(open)
SYSCALL(mknod)
SYSCALL(unlink)
SYSCALL(fstat)
SYSCALL(link)
SYSCALL(mkdir)
SYSCALL(chdir)
SYSCALL(dup)
SYSCALL(getpid)
SYSCALL(sbrk)
SYSCALL(sleep)
SYSCALL(uptime)
SYSCALL(waitpid)
SYSCALL(setpriority)
SYSCALL(getpriority)
SYSCALL(count)
SYSCALL(exitinfo)
SYSCALL(schedinfoinit)
SYSCALL(schedinfo)
SYSCALL(clone)
SYSCALL(yield)
SYSCALL(sem_init)
SYSCALL(sem_wait)
SYSCALL(sem_signal)
SYSCALL(sem_broadcast)
SYSCALL(test_init)
SYSCALL(update_num)
SYSCALL(get_num)
