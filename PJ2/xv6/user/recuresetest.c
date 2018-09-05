#include "types.h"
#include "user.h"
int ppid;
#define assert(x) if (x) {} else { \
   printf(1, "%s: %d ", __FILE__, __LINE__); \
   printf(1, "assert failed (%s)\n", # x); \
   printf(1, "TEST FAILED\n"); \
   kill(ppid); \
   exit(); \
}
int global_value = 1;
void thread2(void* ar) {
  int* arp = (int*)ar;
  arp[0] += 13;
  arp[1] -= 100;
  printf(1, "%d %d\n", arp[0], arp[1]);
  global_value = -15;
  exit();
}
void thread1(void* a) {
  int ar[2];
  int tid = 0;
  int join_id;
  ar[0] = *(int*)a;
  ar[1] = 15;

  printf(1, "%d %d\n", ar[0], ar[1]);
  global_value = 12;
  tid = thread_create(thread2, ar);
  printf(1, "%d\n", global_value);
  join_id = join(tid);
  printf(1, "%d %d %d\n", ar[0], ar[1], global_value);
  assert(ar[0] == *(int*)a + 13 && ar[1] == -85);
  exit();
}

int main() {
  ppid = getpid();
  int value = 1210;
  int ttid = thread_create(thread1, &value);
  join(ttid);
  assert(global_value == -15);
  printf(1, "TEST PASSED\n");
  exit();
}