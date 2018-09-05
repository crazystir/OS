#include "types.h"
#include "user.h"

#define O_RDONLY 0x000
#define O_WRONLY 0x001
#define O_RDWR 0x002
#define O_CREATE 0x200

int
main(int argc, char *argv[])
{
  int fd = open("ls", O_RDONLY);
  int fd2 = open("tests/tests2", O_RDONLY);
  printf(1, "%d\n", fd2);
  char* key = "type";
  char buffer[18];
  int res = getFileTag(fd, key, buffer, 18);
  if(res > 0 && res < 18){
    printf(1, "%s: %s\n", key, buffer);  // prints "type: utility" (assuming tagFile
    // was previously used to set the tag value as "utility"
  } else if (res <= 0) {
    printf(1, "Fail. Please check whether the tag name exists\n");
  } else {
    printf(1, "buffer too small.\n");
  }

  res = getFileTag(fd2, key, buffer, 18);
  if(res > 0 && res < 18){
    printf(1, "%s: %s\n", key, buffer);  // prints "type: utility" (assuming tagFile
    // was previously used to set the tag value as "utility"
  } else if (res <= 0) {
    printf(1, "Fail. Please check whether the tag name exists\n");
  } else {
    printf(1, "buffer too small.\n");
  }


  close(fd);
  close(fd2);

  exit();
}