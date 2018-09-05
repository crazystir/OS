#include "types.h"
#include "x86.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "proc.h"
#include "sysfunc.h"

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return proc->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = proc->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;
  \
  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(proc->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since boot.
int
sys_uptime(void)
{
  uint xticks;
  
  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

int
sys_clone(void)
{
  int fcn;
  int arg, stack;
  int pid;

  if (argint(0, &fcn) < 0 || argint(1, &arg) < 0 || argint(2, &stack) < 0)
    return -1;
  if (validate_stack(proc, (void*)stack) < 0)
    return -1;
  if ((pid = clone((void*)fcn, (void*)arg, (void*)stack)) < 0)
    return -1;
  return pid;
}

int
sys_join(void)
{
  int pid;

  if (argint(0, &pid) < 0)
    return -1;
  if (pid <= 0 || proc->pid == pid)
    return -1;
  if (join(pid) < 0)
    return -1;
  return pid;
}

int
sys_sleep_with_condition(void)
{
  int condition_variable;
  int lock;

  if (argint(0, &condition_variable) < 0 || argint(1, &lock) < 0)
    return -1;
  //Used for test
  ((cond_t*)condition_variable)->signal = 1;
  ((cond_t*)condition_variable)->expectedpid = SIGNAL_ALL;
  sleep_with_condition((cond_t*)condition_variable, (lock_t*)lock);
  return 0;
}

int
sys_wakeup_with_condition(void)
{
  int condition_variable;

  if (argint(0, &condition_variable) < 0)
    return -1;
  ((cond_t*)condition_variable)->signal = 1;
  ((cond_t*)condition_variable)->expectedpid = SIGNAL_ALL;
  wakeup_with_condition((cond_t*)condition_variable);
  return 0;
}
