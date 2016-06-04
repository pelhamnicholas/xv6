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

int get_option(int, char **);
int show_help(void);
int threadyield(void);
int water(void);
int monkeys(void);
int boat(void);
int null_test(void);
int stack(void);
int grow_stack(void);

int main(int argc, char * argv[]) {
  unsigned char option = 0;

  if ((argc < 2) || (option = get_option(argc, argv)) == 0)
    show_help();

  printlock_init();

  printf(1, "\nStarting...\n", option);

  if (option & 0x01) threadyield();
  if (option & 0x02) water();
  if (option & 0x04) monkeys();
  if (option & 0x08) boat();
  if (option & 0x10) null_test();
  if (option & 0x20) stack();
  if (option & 0x40) grow_stack();
  if (option & 0x8000) show_help();

  printf(1, "\nTesting Complete\n");
  
  exit(0);
}

int get_option(int argc, char * argv[]) {
  int option = 0;
  int i;

  if (argc == 1)
    return 0x80;

  for (i = 0; i < argc; i++) {
    if (strcmp(argv[i], "-threadyield") == 0) option |= 0x01;
    else if (strcmp(argv[i], "-water") == 0) option |= 0x02;
    else if (strcmp(argv[i], "-monkeys") == 0) option |= 0x04;
    else if (strcmp(argv[i], "-boat") == 0) option |= 0x08;
    else if (strcmp(argv[i], "-null") == 0) option |= 0x10;
    else if (strcmp(argv[i], "-stack") == 0) option |= 0x20;
    else if (strcmp(argv[i], "-growstack") == 0) option |= 0x40;

    else if (strcmp(argv[i], "-help") == 0) option |= 0x8000;
  }

  return option;
}

int show_help(void) {
    printf(1, "Valid options are:\n");
    printf(1, "  -thradyield      : Tests the functionality of thread_yield\n");
    printf(1, "                     by creating two threads that loop over\n");
    printf(1, "                     a print statement. Two tests are run.\n");
    printf(1, "                     one with a call to thread_yield and one\n");
    printf(1, "                     without.\n");
    printf(1, "  -water           : Tests my implementation of problem 1 in\n");
    printf(1, "                     the lab requirements by creating one of\n");
    printf(1, "                     two threads for hydrogren or oxygen and\n");
    printf(1, "                     making water at appropriate times. The\n");
    printf(1, "                     threads created are chosen randomly.\n");
    printf(1, "  -monkeys         : Tests my implementation of problem 2 in\n");
    printf(1, "                     the lab requirements by creating one of\n");
    printf(1, "                     two threads for a monkey or a dominant\n");
    printf(1, "                     monkey and restricting their ability to\n");
    printf(1, "                     climb a single tree. Threads created \n");
    printf(1, "                     are chosen randomly.\n");
    printf(1, "  -boat            : Tests my implementation of problem 3 in\n");
    printf(1, "                     the lab requirements by creating one\n");
    printf(1, "                     of two threads for a cannibal or a\n");
    printf(1, "                     missionary and arranging them into safe\n");
    printf(1, "                     boats. The threads created are chosen\n");
    printf(1, "                     randomly.\n");
    printf(1, "  -null            : Tests part 2.1 of the lab requirements\n");
    printf(1, "                     by dereferencing a pointer to 0, which\n");
    printf(1, "                     is by standard inaccessible to be used\n");
    printf(1, "                     as NULL.\n");
    printf(1, "  -stack           : Tests the stack rearrangement of part\n");
    printf(1, "                     2.2 by dereferencing a null pointer\n");
    printf(1, "                     to cause a pagefault that prints an\n");
    printf(1, "                     error that includes information about\n");
    printf(1, "                     the stack, in particular stack_top.\n");
    printf(1, "  -growstack       : Tests the ability of the stack to grow\n");
    printf(1, "                     dynamically by calling a recursive\n");
    printf(1, "                     function that pushes data to the stack\n");
    printf(1, "                     until the program crashes with a\n");
    printf(1, "                     pagefault. The pagefault prints an\n");
    printf(1, "                     error with information about the stack,\n");
    printf(1, "                     inparticular stack_top.\n");
    printf(1, "  -help            : Prints this menu.\n");
    exit(0);
}

// Simple, not good, random number generator
int s;
void seed(void) {
  s = uptime();
}

int random(int i, int n) {
  long j;
  s = s + 7 * (i+1) % 101;
  j = (s-1) % n;
  return j;
}

// thread_yield tests

void noyield_test(void* arg) {
  int i;

  for (i = 0; i < 5; i++) {
    printf(1, "%d running\n", getpid());
  }
}

void tyield_test(void* arg) {
  int i;

  for (i = 0; i < 5; i++) {
    printf(1, "%d running\n", getpid());
    thread_yield(0);
  }
}

int threadyield(void) {
  int i;

  printf(1, "Running two processes without thread_yield use.\n");
  printf(1, "These processes can print repeatedly before being scheduled.\n\n");

  for (i = 0; i < 2; i++) {
    thread_create(noyield_test, 0);
  }

  while(wait(0)>=0) ;

  printf(1, "\nRunning two processes with thread_yield use.\n");
  printf(1, "These processes should only print once before ");
  printf(1, "being scheduled.\n\n");

  for (i = 0; i < 2; i++) {
    thread_create(tyield_test, 0);
  }

  while(wait(0)>=0) ;

  return 0;
}

// Problem 1
semaphore H, hRdy, O;

void hReady(void * argv) {
  int * hyd = (int*)argv;
  int * oxy = &hyd[1];
  int * water = &hyd[2];

  //sem_wait(&H);
  //sem_signal(&hRdy);
  sem_wait(&O);
  (*hyd)++;
  if (*hyd >= 2 && *oxy >= 1) {
    (*water)++;
    (*hyd) -= 2;
    (*oxy) -= 1;
    printf(1, "Water created. Molecules left: %d hydrogren %d oxygen\n",
        *hyd, *oxy);
  }
  sem_signal(&O);
}

void oReady(void * argv) {
  int * hyd = (int*)argv;
  int * oxy = &hyd[1];
  int * water = &hyd[2];

  //sem_wait(&O);
  //sem_wait(&hRdy);
  //sem_wait(&hRdy);
  //(*water)++;
  //sem_signal(&O);
  //sem_signal(&H);
  //sem_signal(&H);
  sem_wait(&O);
  (*oxy)++;
  if (*hyd >= 2 && *oxy >= 1) {
    (*water)++;
    (*hyd) -= 2;
    (*oxy) -= 1;
  }
  sem_signal(&O);
}

int water(void) {
  int argv[3];
  int i;//, water;
  int * hyd = argv;
  int * oxy = argv+1;
  int * water = argv+2;

  *hyd = 0;
  *oxy = 0;
  *water = 0;

  printf(1, "Water begin: %d\n", *water);

  sem_init(&H, 2);
  sem_init(&hRdy, 0);
  sem_init(&O, 1);

  for(i = 0; i < 36; i++) {
    if (random(i, 3)) thread_create(hReady, argv);
    else thread_create(oReady, argv);
  }

  while(wait(0)>=0) ;

  printf(1, "Water end: %d\n", *water);
  printf(1, "  Hydrogren left: %d\n", *hyd);
  printf(1, "  Oxygen left: %d\n", *oxy);

  return 0;
}

// Problem 2
semaphore mutex1, mutex2, mutex3, tree, climb, sem_monkey, one_monkey;
int movingup, movingdown, dominant, num_dominant_monkeys, num_monkeys;

void monkey(void * arg) {
  num_monkeys++; // waiting

  sem_wait(&one_monkey);
  sem_wait(&sem_monkey);

  sem_wait(&tree);
  num_monkeys--; // waiting
  sem_wait(&mutex1);
  movingup++;
  if (movingup == 1)
    sem_wait(&climb);
  sem_signal(&mutex1);

  sem_signal(&sem_monkey);
  sem_signal(&one_monkey);

  // climb up
  sem_wait(&mutex1);
  movingup--;
  if (movingup == 0)
    sem_signal(&climb);
  sem_signal(&mutex1);
  // get coconut
  printf(1, "\nMonkey getting coconut\n  %d Monkeys waiting\n  %d Dominant Monkeys waiting\n", num_monkeys, num_dominant_monkeys);
  sem_wait(&mutex2);
  movingdown++;
  if (movingdown == 1)
    sem_wait(&climb);
  sem_signal(&mutex2);
  // climb down
  sem_wait(&mutex2);
  movingdown--;
  if (movingdown == 0)
    sem_signal(&climb);
  sem_signal(&mutex2);
  sem_signal(&tree);
}

void dominant_monkey(void *arg) {
  num_dominant_monkeys++; // waiting

  sem_wait(&mutex3);
  dominant++;
  if (dominant == 1)
    sem_wait(&sem_monkey);
  sem_signal(&mutex3);

  sem_wait(&tree);
  num_dominant_monkeys--; // waiting
  sem_wait(&mutex1);
  movingup++;
  if (movingup == 1)
    sem_wait(&climb);
  sem_signal(&mutex1);
  // climb up
  sem_wait(&mutex1);
  movingup--;
  if (movingup == 0)
    sem_signal(&climb);
  sem_signal(&mutex1);
  // get coconut
  printf(1, "\nDominant Monkey getting coconut\n  %d Monkeys waiting\n  %d Dominant Monkeys waiting\n", num_monkeys, num_dominant_monkeys);

  sem_wait(&mutex2);
  movingdown++;
  if (movingdown == 1)
    sem_wait(&climb);
  sem_signal(&mutex2);
  // climb down
  sem_wait(&mutex2);
  movingdown--;
  if (movingdown == 0)
    sem_signal(&climb);
  sem_signal(&mutex2);
  sem_signal(&tree);

  sem_wait(&mutex3);
  dominant--;
  if (dominant == 0)
    sem_signal(&sem_monkey);
  sem_signal(&mutex3);

}

int monkeys(void) {
  int i;
  movingup = 0;
  movingdown = 0;
  dominant = 0;

  sem_init(&mutex1, 1);
  sem_init(&mutex2, 1);
  sem_init(&mutex3, 1);
  sem_init(&tree, 3);
  sem_init(&climb, 1);
  sem_init(&one_monkey, 1);
  sem_init(&sem_monkey, 1);

  seed();
  for (i = 0; i < 37; i++) {
    if (random(i, 4)) thread_create(monkey, 0);
    else thread_create(dominant_monkey, 0);
    if (i%5) thread_yield(0);
  }

  while(wait(0) >= 0)
    ;

  return 0;
}

// Problem 3
semaphore mutex;

void row_boat(void * argv) {
  int * cannibals = (int *) argv;
  int * missionaries = &cannibals[1];

  if (*missionaries == 3) {
    printf(1, "Boat departing with 0 cannibals and 3 missionaries\n");
    *missionaries = 0;
  } else if (*cannibals == 3) {
    printf(1, "Boat departing with 3 cannibals and 0 missionaries\n");
    *cannibals = 0;
  } else {
    printf(1, "Boat departing with 1 cannibals and 2 missionaries\n");
    *cannibals -= 1;
    *missionaries -= 2;
  }

}

void cannibal_arrives(void * argv) {
  int * cannibals = (int *) argv;
  int * missionaries = &cannibals[1];

  sem_wait(&mutex);
  (*cannibals)++;
  if (*cannibals == 3 || (*cannibals > 0 && *missionaries == 2))
    row_boat(argv);
  sem_signal(&mutex);
}

void missionary_arrives(void * argv) {
  int * cannibals = (int *) argv;
  int * missionaries = &cannibals[1];

  sem_wait(&mutex);
  (*missionaries)++;
  if (*missionaries == 3 || (*cannibals > 0 && *missionaries == 2))
    row_boat(argv);
  sem_signal(&mutex);
}

int boat(void) {
  int argv[2];
  int * cannibals = argv;
  int * missionaries = (argv+1);
  int i;

  *cannibals = 0;
  *missionaries = 0;

  sem_init(&mutex, 1);

  seed();
  for (i = 0; i < 39; i++) {
    if (random(i, 2)) thread_create(missionary_arrives, argv);
    else thread_create(cannibal_arrives, argv);
  }

  while(wait(0) >= 0)
    ;

  printf(1, "Cannibals: %d\nMissionaries: %d\n", *cannibals, *missionaries);

  return 0;
}

// Null

void null(void* v) {
  int *p;
  p = (void *) 0x0;
  printf(1, "%d: p = %x\n", getpid(), *p);
  printf(1, "Null test failed\n");
}

int null_test(void) {
  thread_create(null, (void*) 0);

  wait(0);
  return 0;
}

// stack test

int stack(void) {
  int pid;

  pid = fork();
  if (pid == 0)
    exec("null", 0);
  else if (pid < 0) {
    printf(1, "fail\n");
    exit(1);
  }

  wait(0);
  return 0;
}

// grow stack

int grow_stack(void) {
  int pid;

  pid = fork();
  if (pid == 0)
    exec("stacktest", 0);
  else if (pid < 0) {
    printf(1, "fail\n");
    exit(1);
  }

  wait(0);
  return 0;
}
