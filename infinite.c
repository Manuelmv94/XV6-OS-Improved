#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  char priority;
  priority = atoi(argv[1]);
  set_priority(priority);
  for(;;){};
  exit();
}
