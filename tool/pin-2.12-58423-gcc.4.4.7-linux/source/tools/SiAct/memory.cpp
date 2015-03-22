#include "memory.H"
#include "pin.H"
#include "stdio.h"
#include "hierarchy.H"

extern Tuple<PreciseHierarchy,ApproximateHierarchy> * memory;

MEMORY_ADDR_RANGES::MEMORY_ADDR_RANGES(){
	init_stats();
}
void MEMORY_ADDR_RANGES::init_stats(){
	stats.DRAM_LATENCY_SUM = 0;
	stats.MAX_DRAM_LATENCY = 0;
	stats.MIN_DRAM_LATENCY = 0;
	stats.N_FORCE_REFRESHES = 0;
	stats.N_REFRESHES = 0;
}
void MEMORY_ADDR_RANGES::update_stats(float latency){
	if(stats.N_REFRESHES == 0){
		stats.MAX_DRAM_LATENCY = stats.MIN_DRAM_LATENCY = latency;
	}
	stats.DRAM_LATENCY_SUM+=latency;
	stats.N_REFRESHES++;
	if(stats.MAX_DRAM_LATENCY < latency) stats.MAX_DRAM_LATENCY = latency;
	if(stats.MIN_DRAM_LATENCY > latency) stats.MIN_DRAM_LATENCY = latency;
}
void MEMORY_ADDR_RANGES::print_stats(FILE * out){
	fprintf(out,"Number Refreshes: %ld\n", stats.N_REFRESHES);
	fprintf(out,"Number Full Refreshes: %ld\n", stats.N_FORCE_REFRESHES);
	fprintf(out,"Min DRAM Refresh Rate: %f msec\n", stats.MIN_DRAM_LATENCY);
	fprintf(out,"Max DRAM Refresh Rate: %f msec\n", stats.MAX_DRAM_LATENCY);
	fprintf(out,"Avg DRAM Refresh Rate: %f msec\n", (stats.DRAM_LATENCY_SUM)/stats.N_REFRESHES);
}



VOID mem_urel_alloc(UINT64 address, UINT32 length){
	if(memory->hasSecond())
		memory->second()->alloc(address, length, true);
}

void MEMORY_ADDR_RANGES::report(FILE * out){
	UINT64 UREL_DATA_AMT = 0;
	for(uint32_t i=0; i < addrs.size(); i++){
		UREL_DATA_AMT += addrs[i].end - addrs[i].start;
	}
	printf("Unreliable Data: %f KB", ((float) UREL_DATA_AMT)/1024);
	print_stats(out);
}
#define MAX_RANGE_SIZE 16

void MEMORY_ADDR_RANGES::def(UINT64 start, UINT32 length){
	UINT32 chunks = (length+MAX_RANGE_SIZE -1)/MAX_RANGE_SIZE;
	UINT32 amt = length > MAX_RANGE_SIZE ? MAX_RANGE_SIZE : length;
	UINT32 offset = 0;
	for(UINT32 i =0; i < chunks; i++){
		addr_range_t a;
		a.start = start + offset;
		a.end = start+offset+length;
		if(!contains(start)) addrs.push_back(a);
		//update length
		length -= amt;
		offset += amt;
		amt = length > MAX_RANGE_SIZE ? MAX_RANGE_SIZE : length;
	}
	
}
void MEMORY_ADDR_RANGES::undef(UINT64 addr){
	int idx = find(addr);
	if(idx >= 0){
		addrs.erase(addrs.begin() + idx);
	}
}
int MEMORY_ADDR_RANGES::find(UINT64 addr){
	for(uint32_t i=0; i < addrs.size(); i++){
		if(addrs[i].start <= addr && addrs[i].end > addr) return i;
	}
	return -1;
}
bool MEMORY_ADDR_RANGES::contains(UINT64 addr){
	return (find(addr) >= 0);
}

void MEMORY_ADDR_RANGES::refresh_all(){
	for(uint32_t i=0; i < addrs.size(); i++){
		update_stats(addrs[i].msec);
		addrs[i].msec = 0;
	}
	stats.N_FORCE_REFRESHES++;
}
void MEMORY_ADDR_RANGES::refresh(UINT64 addr){
	int idx = find(addr);
	if(idx >= 0){
		update_stats(addrs[idx].msec);
		addrs[idx].msec = 0;
	}
}
void MEMORY_ADDR_RANGES::accumulate(float msec){
	for(uint32_t i=0; i < addrs.size(); i++){
		addrs[i].msec += msec;
	}
}
float MEMORY_ADDR_RANGES::elapsed(UINT64 addr){
	int idx = find(addr);
	if(idx >= 0){
		return addrs[idx].msec;
	}
	return 0;
}

void MEMORY::Report(FILE * out){
	
}
void MEMORY::Description(FILE * out){
	
}
