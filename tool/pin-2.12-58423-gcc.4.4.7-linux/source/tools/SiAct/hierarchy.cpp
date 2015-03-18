#include "strategy.h"
#include "hierarchy.H"
#include "pin.H"
#include "stdio.h"
#include "cache.H"
#include "approx.H"

Tuple<PreciseHierarchy,ApproximateHierarchy>* memory;
 

PreciseHierarchy::PreciseHierarchy(UINT32 l1i_cachesize, UINT32 l1i_linesize, UINT32 l1i_assoc,
	UINT32 l1d_cachesize, UINT32 l1d_linesize, UINT32 l1d_assoc,
	UINT32 l2_cachesize, UINT32 l2_linesize, UINT32 l2_assoc){
	L1I = new CACHE<LRU<64>, 16*KILO, STORE_ALLOCATE>("L1I",l1i_cachesize, l1i_linesize, l1i_assoc);
	L1I = new (CACHE_LRU(16 * KILO, 64, STORE_ALLOCATE))("L1I",l1i_cachesize, l1i_linesize, l1i_assoc);
	L1D = new CACHE_LRU(16 * KILO, 64, STORE_ALLOCATE)("L1D",l1d_cachesize, l1d_linesize, l1d_assoc);
	L2 = new CACHE_LRU(64 * KILO, 64, STORE_ALLOCATE)("L2",l2_cachesize, l2_linesize, l2_assoc);
	
	MEM = new MEMORY();
	
}
void PreciseHierarchy::load(ADDRINT * data, UINT32 size){
	
}
void PreciseHierarchy::store(ADDRINT * data, UINT32 size){
	
}
void PreciseHierarchy::report(){
	
}

void PreciseHierarchy::description(){
	printf("Precise Hierarchy\n");
	
}


ApproximateHierarchy::ApproximateHierarchy(UINT32 l1i_cachesize, UINT32 l1i_linesize, UINT32 l1i_assoc,
	UINT32 l1d_cachesize, UINT32 l1d_linesize, UINT32 l1d_assoc,
	UINT32 l2_cachesize, UINT32 l2_linesize, UINT32 l2_assoc,
	ApproximateCacheModel L1m, ApproximateCacheModel L2m, ApproximateMemoryModel Mm){
	L1I = new CACHE_LRU(16 * KILO, 64, STORE_ALLOCATE)("L1I",l1i_cachesize, l1i_linesize, l1i_assoc);
	L1D = new APPROX_CACHE_LRU(16 * KILO, 64, STORE_ALLOCATE)("L1D",l1d_cachesize, l1d_linesize, l1d_assoc, L1m);
	L2 = new APPROX_CACHE_LRU(64 * KILO, 64, STORE_ALLOCATE)("L2",l2_cachesize, l2_linesize, l2_assoc, L2m);
	MEM = new APPROXMEMORY(Mm);
}
void ApproximateHierarchy::load(ADDRINT * data, UINT32 size){
	
}
void ApproximateHierarchy::store(ADDRINT * data, UINT32 size){
	
}
void ApproximateHierarchy::alloc(ADDRINT * data, UINT32 size){
	
}
void ApproximateHierarchy::report(){
	
}
void ApproximateHierarchy::elapsed(UINT64 msec){
	
}
void ApproximateHierarchy::description(){
	printf("Approximate Hierarchy\n");
	printf("===== Caches =====\n");
	L1I->Description();
	printf("-----------------\n");
	L1D->Description();
	printf("-----------------\n");
	L2->Description();
	printf("-----------------\n");
	printf("===== Main Memory =====\n");
	MEM->Description();
	printf("-----------------\n");
}
