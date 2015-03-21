#include "memory.H"
#include "pin.H"
#include "stdio.h"
#include "hierarchy.H"

extern Tuple<PreciseHierarchy,ApproximateHierarchy> * memory;

VOID mem_urel_alloc(UINT64 address, UINT32 length){

	printf("[UMEM] unreliable memory allocation: %lx %d\n", address, length);	
	if(memory->hasSecond())
		memory->second()->alloc(address, length, true);
	else
		printf("[UMEM|ERROR] Failed to allocate. Approximate machine not used.\n");
	printf("[UMEM] allocated.\n");
}
void MEMORY_ADDR_RANGES::def(UINT64 start, UINT32 length){
	addr_range_t a;
	a.start = start;
	a.end = start+length;
	addrs.push_back(a);
}
void MEMORY_ADDR_RANGES::undef(UINT64 addr){
	for(uint32_t i=0; i < addrs.size(); i++){
		if(addrs[i].start <= addr && addrs[i].end > addr){
			addrs.erase(addrs.begin() + i);
			return;
		}
			
	}
}
bool MEMORY_ADDR_RANGES::contains(UINT64 addr){
	for(uint32_t i=0; i < addrs.size(); i++){
		if(addrs[i].start <= addr && addrs[i].end > addr) return true;
	}
	return false;
}


void MEMORY::Report(FILE * out){
	
}
void MEMORY::Description(FILE * out){
	
}
