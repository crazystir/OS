/*Two threads modify a single global values, to see whether we got the right answer*/
#include "types.h"
#include "user.h"

lock_t lock;
int global_value;
void
thread1(void* arg)
{
  lock_acquire(&lock);
  global_value += 11;
  lock_release(&lock);
  exit();
}

void
thread2(void* arg)
{
  lock_acquire(&lock);
  global_value += 3;
  lock_release(&lock);
  exit();
}

int
main()
{
  int i, pid1, pid2, count;

  printf(1, "Thread test begin:...\n");
  global_value = 0;
  count = 100;
  lock_init(&lock);
  for (i = 0; i < count; i++) {
    pid1 = thread_create(thread1, 0);
    pid2 = thread_create(thread2, 0);
    thread_join(pid1);
    thread_join(pid2);
  }
  if (global_value != count * 14)
    printf(1, "Test fail. The value is: %d, expected value is: %d\n", global_value, count * 14);
  else
    printf(1, "TEST PASSED\n");
  exit();
}