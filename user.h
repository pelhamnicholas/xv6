struct stat;
struct rtcdate;
//struct semaphore;

// system calls
int fork(void);
int exit(int) __attribute__((noreturn));
int exitinfo(int) __attribute((noreturn));
int wait(int*);
int pipe(int*);
int write(int, void*, int);
int read(int, void*, int);
int close(int);
int kill(int);
int exec(char*, char**);
int open(char*, int);
int mknod(char*, short, short);
int unlink(char*);
int fstat(int fd, struct stat*);
int link(char*, char*);
int mkdir(char*);
int chdir(char*);
int dup(int);
int getpid(void);
char* sbrk(int);
int sleep(int);
int uptime(void);
int waitpid(int, int*, int);
int setpriority(int);
int getpriority(void);
int count(void);
void schedinfoinit(void);
void schedinfo(void);
int clone(void *);
void thread_exit(int);
void sem_init(semaphore *, unsigned int);
void sem_wait(semaphore *);
void sem_signal(semaphore *);
void sem_broadcast(semaphore *);
//
void test_init(void);
int update_num(int);
int get_num(void);

// ulib.c
int stat(char*, struct stat*);
char* strcpy(char*, char*);
void *memmove(void*, void*, int);
char* strchr(const char*, char c);
int strcmp(const char*, const char*);
void printf(int, char*, ...);
char* gets(char*, int max);
uint strlen(char*);
void* memset(void*, int, uint);
void* malloc(uint);
void free(void*);
int atoi(const char*);

// thread.c
int thread_create(void(*)(void*), void *);

// semaphore.c
//void sem_init(semaphore *, uint);
//void sem_wait(semaphore *);
//void sem_signal(semaphore *);
