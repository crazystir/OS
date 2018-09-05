#include "types.h"
#include "user.h"

#define O_RDONLY 0x000
#define O_WRONLY 0x001
#define O_RDWR 0x002
#define O_CREATE 0x200
void
process1() {
  int fd = open("ls", O_RDWR);
  int counter = 1;
  char* key = "type";
  char* val = "utility";
  char* key2 = "Hehe";
  char* val2 = "TianHao Zhang@#!";
  char* key3 = "author";
  char* val3 = "Bill Gates";
  int len = 7;
  int len2 = 16;
  int len3 = 10;
  printf(1, "%d\n", counter++);
  int res = tagFile(fd, key, val, len); // tag file as "type": "utility"
  if(res < 0){
    printf(1, "tagFile error.\n");
  }
  printf(1, "%d\n", counter++);
  int res2 = tagFile(fd, key2, val2, len2);

  if (res2 < 0) {
    printf(1, "tagFile error.\n");
  }
  printf(1, "%d\n", counter++);
  if (tagFile(fd, key3, val3, len3) < 0) {
    printf(1, "tagFile error.\n");
  }
  exit();
}

void
process2() {
  int fd = open("ls", O_RDWR);
  int counter = 1;
  int i;
  char* key = "type";
  char* val = "utility";
  char* key2 = "Hehe";
  char* val2 = "TianHao Zhang@#!";
  char* key3 = "author";
  char* val3 = "Bill Gates";
  char* key4 = "Title";
  char* val4 = "Computer And Art";
  char* key5 = "test0";
  char* val5 = "value0";
  char* key6 = "language";
  char* val6 = "English";
  char* val62 = "Java";
  int len = 7;
  int len2 = 16;
  int len3 = 10;
  int len4 = 16;
  int len5 = 6;
  int len6 = 7;
  int len62 = 4;
  printf(1, "%d\n", counter++);
  int res = tagFile(fd, key, val, len); // tag file as "type": "utility"
  if(res < 0){
    printf(1, "tagFile error.\n");
  }
  printf(1, "%d\n", counter++);
  int res2 = tagFile(fd, key2, val2, len2);

  if (res2 < 0) {
    printf(1, "tagFile error.\n");
  }
  printf(1, "%d\n", counter++);
  res = tagFile(fd, key, val2, len2);
  tagFile(fd, key3, val3, len3);
  tagFile(fd, key2, val3, len3);
  tagFile(fd, key4, val4, len4);
  tagFile(fd, key3, val, len);
  tagFile(fd, key, val4, len4);
  for(i = 0; i < 10; i++) {
    key5[4] = '0' + i;
    val5[5] = '0' + i;
    tagFile(fd, key5, val5, len5);
  }
  tagFile(fd, key6, val6, len6);
  tagFile(fd, key3, val2, len2);
  tagFile(fd, key6, val62, len62);
  close(fd);
  exit();
}

int
main(int argc, char *argv[])
{
  char buffer[18];
  char* key = "type";
  char* key2 = "Hehe";
  char* key3 = "author";
  int length;

  int pid = fork();
  if (pid == 0) {
    process1();
  }
  pid = fork();
  if (pid == 0) {
    process2();
  }
  wait();
  wait();
  int fd = open("ls", O_RDONLY);
  length = getFileTag(fd, key, buffer, 18);
  buffer[length] = 0;
  printf(1, "%d %s\n", length, buffer);
  length = getFileTag(fd, key2, buffer, 18);
  buffer[length] = 0;
  printf(1, "%d %s\n", length, buffer);
  length = getFileTag(fd, key3, buffer, 18);
  buffer[length] = 0;
  printf(1, "%d %s\n", length, buffer);
  close(fd);
  exit();
}