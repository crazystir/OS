/*Main thread exits while child threads are still running*/
#include "types.h"
#include "user.h"


void thread1(void* arg) {
  int i = 0;

  while (i < 10000) {
    printf(1, "child: %d\n", i);
    i++;
  }
  exit();
}

int main() {
  int i = 0;

  thread_create(thread1, 0);
  thread_create(thread1, 0);
  thread_create(thread1, 0);
  while (i < 100) {
    printf(1, "parent: %d\n", i);
    i++;
  }
  printf(1, "TEST PASSED\n");
  exit();
}