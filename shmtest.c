#include "types.h"
#include "user.h"

int main(int argc, char *argv[]){
	uint key=1;
	char *mem;

	mem = (char *)shm_alloc(key,1);

	//checkf if already there is some data
	if(mem[0] != 0){
		printf(1,"%s\n",mem);
	}
	else{
		strcpy(mem,argv[1]);
	}
	shm_free(key);

	exit();
}
