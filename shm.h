#include "types.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "memlayout.h"
#include "x86.h"
#include "traps.h"
#include "proc.h"

#define MAXBLCK 10

typedef struct{
  uint count;
  uint addr;
  uint size;
}block;

block share_mem[MAXBLCK];
