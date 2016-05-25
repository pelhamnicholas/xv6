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

  if (option & 0x01) water();
  if (option & 0x02) monkeys();
  if (option & 0x04) boat();
  if (option & 0x08) null_test();
  if (option & 0x10) stack();
  if (option & 0x20) grow_stack();

  printf(1, "\nTesting Complete\n");
  
  exit(0);
}

int get_option(int argc, char * argv[]) {
  int option = 0;
  int i;

  if (argc == 1)
    return 0x80;

  for (i = 0; i < argc; i++) {
    if (strcmp(argv[i], "-water") == 0) option |= 0x01;
    else if (strcmp(argv[i], "-monkeys") == 0) option |= 0x02;
    else if (strcmp(argv[i], "-boat") == 0) option |= 0x04;
    else if (strcmp(argv[i], "-null") == 0) option |= 0x08;
    else if (strcmp(argv[i], "-stack") == 0) option |= 0x10;
    else if (strcmp(argv[i], "-growstack") == 0) option |= 0x20;
  }

  return option;
}

int show_help(void) {
    printf(1, "Incorrect options. Valid choices are:\n");
    printf(1, "  -water           : Tests\n");
    printf(1, "  -monkeys         : Tests\n");
    printf(1, "  -boat            : Tests\n");
    printf(1, "  -null            : Tests\n");
    printf(1, "  -stack           : Tests\n");
    printf(1, "  -growstack       : Tests\n");
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

// Problem 1
semaphore H, hRdy, O;

void hReady(void * arg) {
  sem_wait(&H);
  sem_signal(&hRdy);
}

void oReady(void * w) {
  int * water = (int*)w;

  sem_wait(&O);
  sem_wait(&hRdy);
  sem_wait(&hRdy);
  (*water)++;
  sem_signal(&O);
  sem_signal(&H);
  sem_signal(&H);
}

int water(void) {
  int i, water;

  water = 0;
  printf(1, "Water begin: %d\n", water);

  sem_init(&H, 2);
  sem_init(&hRdy, 0);
  sem_init(&O, 1);

  for(i = 0; i < 36; i++) {
    if (i%3) thread_create(hReady, 0);
    else thread_create(oReady, &water);
  }

  while(wait(0)>=0) ;

  printf(1, "Water end: %d\n", water);

  return 0;
}

// Problem 2
semaphore mutex1, mutex2, mutex3, tree, climb, sem_monkey, one_monkey;
int movingup, movingdown, dominant, num_monkeys;

void monkey(void * arg) {
  printf(1, "Monkey arrives\n");

  sem_wait(&one_monkey);
  sem_wait(&sem_monkey);

  sem_wait(&tree);
  sem_wait(&mutex1);
  movingup++;
  if (movingup == 1)
    sem_wait(&climb);
  sem_signal(&mutex1);

  sem_signal(&sem_monkey);
  sem_signal(&one_monkey);

  // climb up
  //printf(1, "Monkey climbing up\n");
  sem_wait(&mutex1);
  movingup--;
  if (movingup == 0)
    sem_signal(&climb);
  sem_signal(&mutex1);
  // get coconut
  printf(1, "Monkey getting coconut\n");
  sem_wait(&mutex2);
  movingdown++;
  if (movingdown == 1)
    sem_wait(&climb);
  sem_signal(&mutex2);
  // climb down
  //printf(1, "Monkey climbing down\n");
  sem_wait(&mutex2);
  movingdown--;
  if (movingdown == 0)
    sem_signal(&climb);
  sem_signal(&mutex2);
  sem_signal(&tree);
}

void dominant_monkey(void *arg) {
  printf(1, "Dominant monkey arrives\n");

  sem_wait(&mutex3);
  dominant++;
  if (dominant == 1)
    sem_wait(&sem_monkey);
  sem_signal(&mutex3);

  sem_wait(&tree);
  sem_wait(&mutex1);
  movingup++;
  if (movingup == 1)
    sem_wait(&climb);
  sem_signal(&mutex1);
  // climb up
  //printf(1, "Dominant monkey climbing up\n");
  sem_wait(&mutex1);
  movingup--;
  if (movingup == 0)
    sem_signal(&climb);
  sem_signal(&mutex1);
  // get coconut
  printf(1, "Dominant monkey getting coconut\n");
  sem_wait(&mutex2);
  movingdown++;
  if (movingdown == 1)
    sem_wait(&climb);
  sem_signal(&mutex2);
  // climb down
  //printf(1, "Dominant monkey climbing down\n");
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
    if (random(i, 10)) thread_create(monkey, 0);
    else thread_create(dominant_monkey, 0);
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

  //printf(1, "Cannibal Arrives\n");

  sem_wait(&mutex);
  (*cannibals)++;
  //printf(1, "%d cannibals waiting\n", *cannibals);
  if (*cannibals == 3 || (*cannibals > 0 && *missionaries == 2))
    row_boat(argv);
  sem_signal(&mutex);
}

void missionary_arrives(void * argv) {
  int * cannibals = (int *) argv;
  int * missionaries = &cannibals[1];

  //printf(1, "Missionary Arrives\n");

  sem_wait(&mutex);
  (*missionaries)++;
  //printf(1, "%d missionaries waiting\n", *missionaries);
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
//  int pid;
  thread_create(null, (void*) 0);
/*
  pid = fork();
  if (pid == 0)
    exec("null", 0);
  else if (pid < 0) {
    printf(1, "fail\n");
    exit(1);
  }
*/
  wait(0);
  printf(1, "Null test complete\n");
  return 0;
}

// stack test

int stack(void) {
  int pid;
  //thread_create(null, (void*) 0);

  pid = fork();
  if (pid == 0)
    exec("null", 0);
  else if (pid < 0) {
    printf(1, "fail\n");
    exit(1);
  }

  wait(0);
  printf(1, "Default stack top printed above\n");
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
  printf(1, "Stack test complete\n");
  return 0;
}
