// system calls and required functionality to test priority
// inheritance on ticketlock.
// Adding priority inheritance to the spinlock in xv6 caused
// errors like failed calls to exec that I could not debug.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "ticketlock.h"

struct ticketlock lk;
int num;

void
test_init(void)
{
  initlock_t(&lk, "testlock");
  num = 0;
}

int
update_num(int i)
{
  acquire_t(&lk);
  for( ; i > 0; i--) {
	num++;
  }
  release_t(&lk);
  return num;
}

int
get_num(void)
{
  acquire_t(&lk);
  release_t(&lk);
  return num;
}
