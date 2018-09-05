#include "types.h"
#include "user.h"

#define O_RDONLY 0x000
#define O_WRONLY 0x001
#define O_RDWR 0x002
#define O_CREATE 0x200

int
main(int argc, char *argv[])
{
  int fd = open("ls", O_RDWR);

  char* key = "type";
  char* key2 = "Title";
  int res = removeFileTag(fd, key); // removes the tag with key "type"
  if(res < 0){
    printf(1, "removeFileTag error.\n");
  }
  removeFileTag(fd, key2);

  close(fd);

  exit();
}