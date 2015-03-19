#include "memory.H"
#include "pin.H"
#include "stdio.h"

VOID mem_urel_alloc(UINT64 address, UINT64 length){
	printf("[UMEM] unreliable memory allocation: %lx %ld\n", address, length);
	
}
