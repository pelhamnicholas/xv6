#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "spinlock.h"

struct {
  struct spinlock lock;
  struct proc proc[NPROC];
  int priority[NPROC];
  struct {
      int num_p;
      int response;
      int running;
      int waiting;
      int turnaround;
  } t_info[MAXPRIORITY+1];
  uint t_infostart;
  uint t_infoend;
} ptable;

static struct proc *initproc;

int nextpid = 1;
extern void forkret(void);
extern void trapret(void);

static void wakeup1(void *chan);

void
pinit(void)
{
  int i;

  initlock(&ptable.lock, "ptable");
  for(i = 0; i < MAXPRIORITY+1; i++)
    ptable.priority[i] = 0;
  ptable.t_infostart = 0;
}

void
schedinfoinit(void)
{
  int i;
  for(i = 0; i < MAXPRIORITY+1; i++) {
    ptable.t_info[i].num_p = 0;
    ptable.t_info[i].response = 0;
    ptable.t_info[i].running = 0;
    ptable.t_info[i].waiting = 0;
    ptable.t_info[i].turnaround = 0;
  }
  ptable.t_infostart = ticks;
}

void
schedinfo(void)
{
  int i;
  ptable.t_infoend = ticks;

  for(i = 0; i < MAXPRIORITY+1; i++) {
    if(ptable.t_info[i].num_p > 0) {
      cprintf("Processes Averaged at Priority %d: %d\n", 
              i, ptable.t_info[i].num_p);
      cprintf("  Response Time:                   %d\n", 
              ptable.t_info[i].response);
      cprintf("  Running Time:                    %d\n", 
              ptable.t_info[i].running);
      cprintf("  Waiting Time:                    %d\n", 
              ptable.t_info[i].waiting);
      cprintf("  Turnaround Time:                 %d\n", 
              ptable.t_info[i].turnaround);
    }
  }
  return;
}

// Set the current process's priority.
int
setpriority (int priority) {
  //acquire(&ptable.lock);
  if(priority > MAXPRIORITY)
    proc->priority = MAXPRIORITY;
  else if(priority < 0)
    proc->priority = 0;
  else
    proc->priority = priority;
  return proc->priority;
  //release(&ptable.lock);
}

int
getpriority(void)
{
    return (int) proc->priority;
}

void
givepriority(struct proc *p)
{
    //acquire(&ptable.lock);
    if(p->priority < proc->priority)
        p->priority = proc->priority;
    //release(&ptable.lock);
}

void
resetpriority()
{
    //acquire(&ptable.lock);
    proc->priority = proc->basepriority;
    //release(&ptable.lock);
}

//PAGEBREAK: 32
// Look in the process table for an UNUSED proc.
// If found, change state to EMBRYO and initialize
// state required to run in the kernel.
// Otherwise return 0.
static struct proc*
allocproc(void)
{
  struct proc *p;
  char *sp;

  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == UNUSED)
      goto found;
  release(&ptable.lock);
  return 0;

found:
  p->state = EMBRYO;
  p->pid = nextpid++;
  release(&ptable.lock);

  // Allocate kernel stack.
  if((p->kstack = kalloc()) == 0){
    p->state = UNUSED;
    return 0;
  }
  sp = p->kstack + KSTACKSIZE;
  
  // Leave room for trap frame.
  sp -= sizeof *p->tf;
  p->tf = (struct trapframe*)sp;
  
  // Set up new context to start executing at forkret,
  // which returns to trapret.
  sp -= 4;
  *(uint*)sp = (uint)trapret;

  sp -= sizeof *p->context;
  p->context = (struct context*)sp;
  memset(p->context, 0, sizeof *p->context);
  p->context->eip = (uint)forkret;

  return p;
}

//PAGEBREAK: 32
// Set up first user process.
void
userinit(void)
{
  struct proc *p;
  extern char _binary_initcode_start[], _binary_initcode_size[];
  
  p = allocproc();
  initproc = p;
  if((p->pgdir = setupkvm()) == 0)
    panic("userinit: out of memory?");
  inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
  p->sz = PGSIZE;
  memset(p->tf, 0, sizeof(*p->tf));
  p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
  p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
  p->tf->es = p->tf->ds;
  p->tf->ss = p->tf->ds;
  p->tf->eflags = FL_IF;
  p->tf->esp = PGSIZE;
  p->tf->eip = 0;  // beginning of initcode.S

  safestrcpy(p->name, "initcode", sizeof(p->name));
  p->cwd = namei("/");

  p->state = RUNNABLE;
}

// Grow current process's memory by n bytes.
// Return 0 on success, -1 on failure.
int
growproc(int n)
{
  uint sz;
  
  sz = proc->sz;
  if(n > 0){
    if((sz = allocuvm(proc->pgdir, sz, sz + n)) == 0)
      return -1;
  } else if(n < 0){
    if((sz = deallocuvm(proc->pgdir, sz, sz + n)) == 0)
      return -1;
  }
  proc->sz = sz;
  switchuvm(proc);
  return 0;
}

// Create a new process copying p as the parent.
// Sets up stack to return as if from system call.
// Caller must set state of returned proc to RUNNABLE.
int
fork(void)
{
  int i, pid;
  struct proc *np;

  // Allocate process.
  if((np = allocproc()) == 0)
    return -1;

  // Copy process state from p.
  if((np->pgdir = copyuvm(proc->pgdir, proc->sz)) == 0){
    kfree(np->kstack);
    np->kstack = 0;
    np->state = UNUSED;
    return -1;
  }
  np->sz = proc->sz;
  np->parent = proc;
  *np->tf = *proc->tf;
  np->basepriority = np->priority = proc->priority; // init priority

  // Clear %eax so that fork returns 0 in the child.
  np->tf->eax = 0;

  for(i = 0; i < NOFILE; i++)
    if(proc->ofile[i])
      np->ofile[i] = filedup(proc->ofile[i]);
  np->cwd = idup(proc->cwd);

  safestrcpy(np->name, proc->name, sizeof(proc->name));
 
  pid = np->pid;

  // lock to force the compiler to emit the np->state write last.
  acquire(&ptable.lock);
  np->state = RUNNABLE;
  np->starttime = ticks;
  np->runtime = 0;
  proc->runtime += temptime - np->starttime;
  temptime = np->starttime;
  release(&ptable.lock);
  
  return pid;
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait() to find out it exited.
void
exit(int status)
{
  struct proc *p;
  int fd;

  proc->exitstatus = status;

  if(proc == initproc)
    panic("init exiting");

  // Close all open files.
  for(fd = 0; fd < NOFILE; fd++){
    if(proc->ofile[fd]){
      fileclose(proc->ofile[fd]);
      proc->ofile[fd] = 0;
    }
  }

  begin_op();
  iput(proc->cwd);
  end_op();
  proc->cwd = 0;

  acquire(&ptable.lock);

  // Parent might be sleeping in wait().
  wakeup1(proc->parent);

  // Wakeup any process sleeping in waitpid().
  wakeup1(proc);

  // Pass abandoned children to init.
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->parent == proc){
      p->parent = initproc;
      if(p->state == ZOMBIE)
        wakeup1(initproc);
    }
  }

  // Time info
  proc->endtime = ticks;
  proc->turnaroundtime = proc->endtime - proc->starttime;
  proc->runtime += proc->endtime - temptime;
  // update ptable.t_info
  if(ptable.t_infostart != 0) {
    ptable.t_info[proc->priority].response =
          (ptable.t_info[proc->priority].response
            * ptable.t_info[proc->priority].num_p + proc->responsetime) 
            / (ptable.t_info[proc->priority].num_p + 1);
    ptable.t_info[proc->priority].running = 
          (ptable.t_info[proc->priority].running 
            * ptable.t_info[proc->priority].num_p
            + proc->runtime) / (ptable.t_info[proc->priority].num_p + 1);
    ptable.t_info[proc->priority].waiting = 
          (ptable.t_info[proc->priority].waiting 
            * ptable.t_info[proc->priority].num_p
            + (proc->turnaroundtime - proc->runtime)) 
            / (ptable.t_info[proc->priority].num_p + 1);
    ptable.t_info[proc->priority].turnaround = 
          (ptable.t_info[proc->priority].turnaround 
            * ptable.t_info[proc->priority].num_p
            + proc->turnaroundtime) / (ptable.t_info[proc->priority].num_p + 1);
    ptable.t_info[proc->priority].num_p++;
  }

  // Jump into the scheduler, never to return.
  proc->state = ZOMBIE;
  sched();
  panic("zombie exit");
}

void
exitinfo(int status)
{
  struct proc *p;
  int fd;

  proc->exitstatus = status;

  if(proc == initproc)
    panic("init exiting");

  // Close all open files.
  for(fd = 0; fd < NOFILE; fd++){
    if(proc->ofile[fd]){
      fileclose(proc->ofile[fd]);
      proc->ofile[fd] = 0;
    }
  }

  begin_op();
  iput(proc->cwd);
  end_op();
  proc->cwd = 0;

  acquire(&ptable.lock);

  // Parent might be sleeping in wait().
  wakeup1(proc->parent);

  // Wakeup any process sleeping in waitpid().
  wakeup1(proc);

  // Pass abandoned children to init.
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->parent == proc){
      p->parent = initproc;
      if(p->state == ZOMBIE)
        wakeup1(initproc);
    }
  }

  // Time info
  proc->endtime = ticks;
  proc->turnaroundtime = proc->endtime - proc->starttime;
  proc->runtime += proc->endtime - temptime;
  // update ptable.t_info
  if(ptable.t_infostart != 0) {
    ptable.t_info[proc->priority].response =
          (ptable.t_info[proc->priority].response
            * ptable.t_info[proc->priority].num_p + proc->responsetime) 
            / (ptable.t_info[proc->priority].num_p + 1);
    ptable.t_info[proc->priority].running = 
          (ptable.t_info[proc->priority].running 
            * ptable.t_info[proc->priority].num_p
            + proc->runtime) / (ptable.t_info[proc->priority].num_p + 1);
    ptable.t_info[proc->priority].waiting = 
          (ptable.t_info[proc->priority].waiting 
            * ptable.t_info[proc->priority].num_p
            + (proc->turnaroundtime - proc->runtime)) 
            / (ptable.t_info[proc->priority].num_p + 1);
    ptable.t_info[proc->priority].turnaround = 
          (ptable.t_info[proc->priority].turnaround 
            * ptable.t_info[proc->priority].num_p
            + proc->turnaroundtime) / (ptable.t_info[proc->priority].num_p + 1);
    ptable.t_info[proc->priority].num_p++;
  }
  // print individual process info
  cprintf("[%d] Start time:      %d\n", proc->pid, proc->starttime);
  cprintf("     End time:        %d\n", proc->endtime);
  cprintf("     Turnaround time: %d\n", proc->turnaroundtime);
  cprintf("     Response time:   %d\n", proc->responsetime);
  cprintf("     Running time:    %d\n", proc->runtime);
  cprintf("     Waiting time:    %d\n", proc->turnaroundtime - proc->runtime);

  // Jump into the scheduler, never to return.
  proc->state = ZOMBIE;
  sched();
  panic("zombie exit");
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int
wait(int *status)
{
  struct proc *p;
  int havekids, pid;

  acquire(&ptable.lock);
  for(;;){
    // Scan through table looking for zombie children.
    havekids = 0;
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->parent != proc)
        continue;
      havekids = 1;
      if(p->state == ZOMBIE){
        // Found one.
        pid = p->pid;
        kfree(p->kstack);
        p->kstack = 0;
        freevm(p->pgdir);
        p->state = UNUSED;
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->killed = 0;
        if (status)
          *status = p->exitstatus;
        release(&ptable.lock);
        return pid;
      }
    }

    // No point waiting if we don't have any children.
    if(!havekids || proc->killed){
      release(&ptable.lock);
      if (status)
        *status = -1;
      return -1;
    }

    // Wait for children to exit.  (See wakeup1 call in proc_exit.)
    sleep(proc, &ptable.lock);  //DOC: wait-sleep
  }
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int
waitpid(int pid, int *status, int option)
{
  struct proc *p;
  int found = 0;

  acquire(&ptable.lock);
  for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
    if(p->pid != pid)
      continue;
    else {
      found = 1;
      break;
    }
  }
  if (!found) {
    release(&ptable.lock);
    if (status)
      *status = -1;
    return -1;
  } else {
    for (;;) {
      if(p->state == ZOMBIE) {
        kfree(p->kstack);
        p->kstack = 0;
        freevm(p->pgdir);
        p->state = UNUSED;
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->killed = 0;
        if (status)
          *status = p->exitstatus;
        release(&ptable.lock);
        return(pid);
      }
      sleep(p, &ptable.lock);
    }
  }
}

//PAGEBREAK: 42
// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run
//  - swtch to start running that process
//  - eventually that process transfers control
//      via swtch back to the scheduler.
void
scheduler(void)
{
  struct proc *p;
  //struct proc *np, *lp;
  int priority;

  for(;;){
    // Enable interrupts on this processor.
    sti();

    // Loop over process table looking for process to run.
    acquire(&ptable.lock);

    priority = 0;
    
    // Find highest priority level in runnable processes
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
      if(p->state != RUNNABLE)
        continue;
      if (priority < p->priority)
        priority = p->priority;
    }

    // Round robin all processes of highest priority
    for ( ; priority >= 0; priority--) {

        for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
          if(p->state != RUNNABLE)
            continue;
          if(p->priority < priority)
            continue;

          // Switch to chosen process.  It is the process's job
          // to release ptable.lock and then reacquire it
          // before jumping back to us.
          proc = p;
          switchuvm(p);
          p->state = RUNNING;
          temptime = ticks;
          if(p->runtime == 0)
            p->responsetime = temptime - p->starttime;
          swtch(&cpu->scheduler, proc->context);
          switchkvm();

          // Process is done running for now.
          // It should have changed its p->state before coming back.
          proc = 0;
        }
    }
    release(&ptable.lock);

  }
}

// Enter scheduler.  Must hold only ptable.lock
// and have changed proc->state.
void
sched(void)
{
  int intena;

  // time debug
  proc->runtime += ticks - temptime;
  // end time debug

  if(!holding(&ptable.lock))
    panic("sched ptable.lock");
  if(cpu->ncli != 1)
    panic("sched locks");
  if(proc->state == RUNNING)
    panic("sched running");
  if(readeflags()&FL_IF)
    panic("sched interruptible");
  intena = cpu->intena;
  swtch(&proc->context, cpu->scheduler);
  cpu->intena = intena;
}

// Give up the CPU for one scheduling round.
void
yield(void)
{
  acquire(&ptable.lock);  //DOC: yieldlock
  proc->state = RUNNABLE;
  sched();
  release(&ptable.lock);
}

// A fork child's very first scheduling by scheduler()
// will swtch here.  "Return" to user space.
void
forkret(void)
{
  static int first = 1;
  // Still holding ptable.lock from scheduler.
  release(&ptable.lock);

  if (first) {
    // Some initialization functions must be run in the context
    // of a regular process (e.g., they call sleep), and thus cannot 
    // be run from main().
    first = 0;
    iinit(ROOTDEV);
    initlog(ROOTDEV);
  }
  
  // Return to "caller", actually trapret (see allocproc).
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void
sleep(void *chan, struct spinlock *lk)
{
  if(proc == 0)
    panic("sleep");

  // Why would this cause panic?
  //if(lk == 0)
  //  panic("sleep without lk");

  // Must acquire ptable.lock in order to
  // change p->state and then call sched.
  // Once we hold ptable.lock, we can be
  // guaranteed that we won't miss any wakeup
  // (wakeup runs with ptable.lock locked),
  // so it's okay to release lk.
  if(lk != &ptable.lock){  //DOC: sleeplock0
    acquire(&ptable.lock);  //DOC: sleeplock1
    if(lk)
      release(lk);
  }

  // Go to sleep.
  proc->chan = chan;
  proc->state = SLEEPING;
  sched();

  // Tidy up.
  proc->chan = 0;

  // Reacquire original lock.
  if(lk != &ptable.lock){  //DOC: sleeplock2
    release(&ptable.lock);
    if(lk)
      acquire(lk);
  }
}

//PAGEBREAK!
// Wake up all processes sleeping on chan.
// The ptable lock must be held.
static void
wakeup1(void *chan)
{
  struct proc *p;

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == SLEEPING && p->chan == chan)
      p->state = RUNNABLE;
}

// Wake up all processes sleeping on chan.
void
wakeup(void *chan)
{
  acquire(&ptable.lock);
  wakeup1(chan);
  release(&ptable.lock);
}

// Kill the process with the given pid.
// Process won't exit until it returns
// to user space (see trap in trap.c).
int
kill(int pid)
{
  struct proc *p;

  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->pid == pid){
      p->killed = 1;
      // Wake process from sleep if necessary.
      if(p->state == SLEEPING)
        p->state = RUNNABLE;
      release(&ptable.lock);
      return 0;
    }
  }
  release(&ptable.lock);
  return -1;
}

//PAGEBREAK: 36
// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
void
procdump(void)
{
  static char *states[] = {
  [UNUSED]    "unused",
  [EMBRYO]    "embryo",
  [SLEEPING]  "sleep ",
  [RUNNABLE]  "runble",
  [RUNNING]   "run   ",
  [ZOMBIE]    "zombie"
  };
  int i;
  struct proc *p;
  char *state;
  uint pc[10];
  
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->state == UNUSED)
      continue;
    if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
      state = states[p->state];
    else
      state = "???";
    cprintf("%d %s %s", p->pid, state, p->name);
    if(p->state == SLEEPING){
      getcallerpcs((uint*)p->context->ebp+2, pc);
      for(i=0; i<10 && pc[i] != 0; i++)
        cprintf(" %p", pc[i]);
    }
    cprintf("\n");
  }
}
