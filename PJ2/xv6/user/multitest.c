/*Multi threads apply for the memory at the same time*/
#include "types.h"
#include "user.h"

#define MAX 2
#define MAX_ITS 2

void memalloc(void* arg) {
  char* p;
  p = malloc(2);
  exit();
}

int main() {
  int i = 0;
  int pid[MAX];
  int count = MAX_ITS;

  printf(1, "Test begin...\n");
  printf(1, "%p\n", malloc(0));
  while (count--) {
    for (i = 0; i < MAX; i++) {
      pid[i] = thread_create(memalloc, 0);
    }
    for (i = 0; i < MAX; ++i) {
      thread_join(pid[i]);
    }
  }
  printf(1, "%p\n", malloc(0));
  printf(1, "TEST PASSED\n");
  exit();
}