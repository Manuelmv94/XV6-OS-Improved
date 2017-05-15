#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
//  int i;

  if(argc != 3){
    printf(2, "Usage: ln old new\n");
    printf(2, "Usage: rm files...\n");
    exit();
  }
  if(link(argv[1], argv[2]) < 0)
    printf(2, "link %s %s: failed\n", argv[1], argv[2]);

  if(unlink(argv[1]) < 0)
    printf(2, "rm: %s failed to delete\n", argv[1]);

  exit();
}
