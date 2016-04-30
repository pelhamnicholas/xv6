// Mutual exclusion lock.
struct ticketlock {
  uint ticket;       // Is the lock held?
  uint turn;

  struct proc *proc;
  
  // For debugging:
  char *name;        // Name of lock.
  struct cpu *cpu;   // The cpu holding the lock.
  uint pcs[10];      // The call stack (an array of program counters)
                     // that locked the lock.
};
