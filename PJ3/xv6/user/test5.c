/* call tagFile to tag a file.  Call getFileTag to read the tag of that file. Call removeFileTag to remove the tag.  Call getFileTag again to verify that the tag is removed. */
#include "types.h"
#include "user.h"

#define O_RDONLY  0x000
#define O_WRONLY  0x001
#define O_RDWR    0x002
#define O_CREATE  0x200

#undef NULL
#define NULL ((void*)0)

int ppid;
volatile int global = 1;

#define assert(x) if (x) {} else { \
   printf(1, "%s: %d ", __FILE__, __LINE__); \
   printf(1, "assert failed (%s)\n", # x); \
   printf(1, "TEST FAILED\n"); \
   kill(ppid); \
   exit(); \
}

void print(int resultsLength, char* results) {
  int p = 0;
  int count = 0;
  printf(1, "%d\n", resultsLength);
  while (count < resultsLength) {
    printf(1, "%s\n", &results[p]);
    while (results[p++]);
    count++;
  }
}

int
main(int argc, char *argv[])
{
  ppid = getpid();
  int fd1 = open("ls", O_RDWR);
  int fd2 = open("cat", O_RDWR);
  int fd3 = open("echo", O_RDWR);
  int fd4 = open("wc", O_RDWR);
  printf(1, "fd of ls: %d %d %d\n", fd1, fd2, fd3);
  char* key = "type";
  char* val = "utility";
  char* key2 = "author";
  char* val2 = "King";
  char results[200];
  int len = 7;
  int len2 = 4;
  int res = tagFile(fd1, key, val, len);
  assert(res > 0);
  res = tagFile(fd2, key, val, len);
  assert(res > 0);
  res = tagFile(fd3, key, val, len);
  assert(res > 0);
  res = tagFile(fd3, key2, val2, len2);
  res = tagFile(fd4, key2, val2, len2);
  char buf[7];
  int valueLength = getFileTag(fd1, key, buf, 7);
  assert(valueLength == len);
  valueLength = getFileTag(fd2, key, buf, 7);
  assert(valueLength == len);
  valueLength = getFileTag(fd3, key, buf, 7);
  assert(valueLength == len);

  int resLength = getFilesByTag(key, val, 7, results, 200);
  print(resLength, results);
  resLength = getFilesByTag(key2, val2, len2, results, 200);
  print(resLength, results);

  close(fd1);
  close(fd2);
  close(fd3);
  close(fd4);
  printf(1, "TEST PASSED\n");
  exit();
}
