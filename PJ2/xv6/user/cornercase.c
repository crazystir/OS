//Create another thread which creates another thread
#include "types.h"
#include "user.h"
#define PGSIZE (4096)
int ppid;
#define assert(x) if (x) {} else { \
   printf(1, "%s: %d ", __FILE__, __LINE__); \
   printf(1, "assert failed (%s)\n", # x); \
   printf(1, "TEST FAILED\n"); \
   kill(ppid); \
   exit(); \
}
int global_value = 1;

void thread3(void* b) {
  *(int*) b = 18;
  global_value += 3;
  exit();
}
void thread2(void* ar) {
  global_value = 13;
  *(int*)ar = 18;
  exit();
}
void thread1(void* a) {
  join(*(int*)a);
  assert(global_value == 13);
  exit();
}

int main() {
  ppid = getpid();
  int count = 30;
  int value = 1210;
  int i = 0;
  void *stack1 = malloc(PGSIZE*2);
  assert(stack1 != NULL);
  if((uint)stack1 % PGSIZE) {
    stack1 = stack1 + (PGSIZE - (uint)stack1 % PGSIZE);
  }

  void *stack2 = malloc(PGSIZE*2);
  assert(stack2 != NULL);
  if((uint)stack2 % PGSIZE) {
    stack1 = stack2 + (PGSIZE - (uint)stack2 % PGSIZE);
  }

  for (i = 0; i < count; i++) {
    int ttid2 = clone(thread2, &value, stack2);
    int ttid1 = clone(thread1, &ttid2, stack1);
    join(ttid1);
    assert(value == 18);
  }
  printf(1, "TEST PASSED\n");
  exit();
}