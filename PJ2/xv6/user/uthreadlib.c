#include "types.h"
#include "user.h"
#include "x86.h"
#include "param.h"

#define PGSIZE (4096)
//A data structure used to manage the allocated memory
struct {
  struct node nodes[NPROC];
} mem_management;

void lock_acquire(lock_t* lock) {
  int pid;

  if (lock == 0)
    return;
  pid = getpid();
  if (lock->pid != pid) {
    while (xchg(&lock->locked, 1) != 0)
      ;
    lock->pid = pid;
  }
}

void lock_release(lock_t* lock) {
  int pid;

  if (lock == 0)
    return;
  pid = getpid();
  if (lock->pid == pid) {
    lock->pid = UNUSED_PID;
    xchg(&lock->locked, 0);
  }
}

void lock_init(lock_t* lock) {
  if (lock == 0)
    return;
  lock->pid = UNUSED_PID;
  lock->locked = 0;
}

//We must assume the thread has the lock
//We must acquire the ptable lock before release or lock
void cv_wait(cond_t* conditionVariable, lock_t* lock) {
  if (conditionVariable == 0|| lock == 0)
    return;
  sleep_with_condition(conditionVariable, lock);
}

void cv_signal(cond_t* conditionVariable) {
  if (conditionVariable == 0)
    return;
  wakeup_with_condition(conditionVariable);
}

void* allocmem() {
  void* addr;

  addr = malloc(PGSIZE*2); // as before, allocate 2 pages
  if((uint)addr % PGSIZE){
    addr = addr + (PGSIZE - (uint)addr % PGSIZE); // make sure the stack is page aligned
  }
  return addr;
}

int mem_search() {
  struct node* n;
  int res = -1;
  int i = 0;

  for (n = mem_management.nodes; n < &mem_management.nodes[NPROC] && res == -1; n++) {
    lock_acquire(&n->lock);
    if (n->free == 0) {
      n->free = 1;
      if (n->p == 0) {
        n->p = allocmem();
      }
      res = i;
    }
    lock_release(&n->lock);
    i++;
  }
  return res;
}

void mem_free(int pid) {
  struct node* n;

  for (n = mem_management.nodes; n < &mem_management.nodes[NPROC]; n++) {
    if (n->id == pid) {
      lock_acquire(&n->lock);
      n->free = 0;
      n->id = -1;
      lock_release(&n->lock);
      break;
    }
  }
}
int thread_create(void (*start_routine)(void*), void* arg) {
  int pid;
  int index;

  index = mem_search();
  pid = clone(start_routine, arg, mem_management.nodes[index].p);
  mem_management.nodes[index].id = pid;
  return pid;
}

int thread_join(int pid) {
  int p;

  p = join(pid);
  if (p < 0)
    return -1;
  mem_free(pid);
  return p;
}