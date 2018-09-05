/*A simple producer - consumer model. I create two producers and two consumers and there is a global resources*/
#include "types.h"
#include "user.h"


#define P_EXPECTED_VALUE 0
#define C_EXPECTED_VALUE 1
lock_t lock;
cond_t p_conditionVariable;
cond_t c_conditionVariable;
cond_t global_signal;
int global_value;
int global_resource;
int global_output;

void
producer1(void* arg)
{
  lock_acquire(&lock);
  while (global_signal.signal != P_EXPECTED_VALUE)
    cv_wait(&p_conditionVariable, &lock);
  global_resource = ++global_value;
  global_signal.signal = C_EXPECTED_VALUE;
  cv_signal(&c_conditionVariable);
  lock_release(&lock);
  exit();
}

void
consumer1(void* arg)
{
  lock_acquire(&lock);
  while (global_signal.signal != C_EXPECTED_VALUE)
    cv_wait(&c_conditionVariable, &lock);
  global_output += global_resource;
  global_signal.signal = P_EXPECTED_VALUE;
  cv_signal(&p_conditionVariable);
  lock_release(&lock);
  exit();
}

int
main()
{
  int i, pid1, pid2, pid3, pid4, count;

  printf(1, "Thread test begin:...\n");
  global_value = 0;
  global_resource = 0;
  global_output = 0;
  global_signal.signal = P_EXPECTED_VALUE;
  p_conditionVariable.signal = P_EXPECTED_VALUE;
  p_conditionVariable.expectedpid = SIGNAL_ALL;
  c_conditionVariable.signal = C_EXPECTED_VALUE;
  c_conditionVariable.expectedpid = SIGNAL_ALL;
  count = 1000;
  lock_init(&lock);
  for (i = 0; i < count; i++) {
    pid1 = thread_create(producer1, 0);
    pid2 = thread_create(consumer1, 0);
    pid3 = thread_create(producer1, 0);
    pid4 = thread_create(consumer1, 0);
    thread_join(pid1);
    thread_join(pid2);
    thread_join(pid3);
    thread_join(pid4);
  }
  if (global_output != (count) * (2 * count + 1))
    printf(1, "Test fail. The value is: %d, expected value is: %d\n", global_output, (count) * (count + 1) / 2);
  else
    printf(1, "TEST PASSED\n");
  exit();
}