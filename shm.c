#include "shm.h"

void init_shm(){
  int i;
  cprintf("init shared memory\n");
  for(i=0;i<MAXBLCK;i++){
    share_mem[i].count=0;
    share_mem[i].addr=0;
    share_mem[i].size=0;
  }

}

extern int mappages(pde_t *pgdir, void *va, uint size, uint pa, int perm);

int sys_shm_alloc(void){
  int key;
  int size;
  int pos;
  uint addr;

  argint(0,&key);
  argint(1,&size);
  pos=key % MAXBLCK;

  if(share_mem[pos].count==0){
    share_mem[pos].addr= (uint)kalloc();
    memset((void*)share_mem[pos].addr,0,PGSIZE);
  }

  share_mem[pos].count+=1;
  addr=PGROUNDUP(proc->sz);
  mappages(proc->pgdir,(char *)addr, PGSIZE,v2p((char *)share_mem[pos].addr), PTE_W | PTE_U);
  proc->sz += PGSIZE;
  switchuvm(proc);
  return addr;
}


extern pte_t * walkpgdir(pde_t *pgdir, const void *va, int alloc);

int sys_shm_free(void){
  int key;
//  int pos;
  uint addr;
  pte_t *pte;

  argint(0,&key);
  addr=PGROUNDUP(proc->sz -PGSIZE);
  pte=walkpgdir(proc->pgdir,(char*)addr,0);
  *pte=0;
  proc->sz -=PGSIZE;

//  pos=key%MAXBLCK;
//  share_mem[pos].count-=1;
  /*if(share_mem[pos].count==0){
    kfree((char*)share_mem[pos].addr);
    share_mem[pos].addr=0;
  }*/
  return 0;

}
