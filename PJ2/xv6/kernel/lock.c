#include "types.h"
#include "x86.h"
#include "param.h"
#include "mmu.h"
#include "proc.h"

void
lock_kacquire(lock_t* lock, int pid)
{
  if (lock == 0)
    return;
  if (lock->pid != pid) {
    while (xchg(&lock->locked, 1) != 0)
      ;
    lock->pid = pid;
  }
}

void
lock_krelease(lock_t* lock)
{
  if (lock == 0)
    return;
  if (lock->pid == proc->pid) {
    lock->pid = UNUSED_PID;
    xchg(&lock->locked, 0);
  }
}

void
lock_kinit(lock_t* lock)
{
  if (lock == 0)
    return;
  lock->pid = UNUSED_PID;
  lock->locked = 0;
}
