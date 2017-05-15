#include "types.h"
#include "stat.h"
#include "user.h"


int
main(int argc, char *argv[])
{
  int fd, perm;
  perm=atoi(argv[1]);
  fd=open(argv[2],0);
  chmod(fd,perm);
  close(fd);
  exit();
}
