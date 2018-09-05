#ifndef _TYPES_H_
#define _TYPES_H_

// Type definitions

typedef unsigned int   uint;
typedef unsigned short ushort;
typedef unsigned char  uchar;
typedef uint pde_t;
#ifndef NULL
#define NULL (0)
#endif

//I store the id to help figuring out who owns the lock
struct userlock {
  int pid;
  uint locked;
};

//The signal is the value I want and the expectedpid is the pid that the thread waits for.
//For example, a main thread can wait for a specific child thread to finish some job and send a signal
struct condition {
  int signal;
  int expectedpid;
};

struct node {
  struct userlock lock;
  int id;
  int free;
  void* p;
};
#define UNUSED_PID 0 //Means the lock is not used
#define SIGNAL_ALL 0 //Means the process accept every process as long as the signal is correct
#define SIGNAL_NONE (-1) //Means the process will not accept any one's signal
typedef struct userlock lock_t;
typedef struct condition cond_t;

void lock_kacquire(lock_t*, int);
void lock_krelease(lock_t*);
void lock_kinit(lock_t*);

#endif //_TYPES_H_
