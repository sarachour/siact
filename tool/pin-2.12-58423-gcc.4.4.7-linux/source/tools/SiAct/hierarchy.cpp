#include "hierarchy.H"
#include "pin.H"

Tuple<PreciseHierarchy,ApproximateHierarchy>* memory;
 

PreciseHierarchy::PreciseHierarchy(UINT32 l1i_cachesize, UINT32 l1i_linesize, UINT32 l1i_assoc,
	UINT32 l1d_cachesize, UINT32 l1d_linesize, UINT32 l1d_assoc,
	UINT32 l2_cachesize, UINT32 l2_linesize, UINT32 l2_assoc){
		
}
void PreciseHierarchy::load(ADDRINT * data, UINT32 size){
	
}
void PreciseHierarchy::store(ADDRINT * data, UINT32 size){
	
}
void PreciseHierarchy::report(){
	
}

void PreciseHierarchy::description(){
	
	
}

ApproximateHierarchy::ApproximateHierarchy(UINT32 l1i_cachesize, UINT32 l1i_linesize, UINT32 l1i_assoc,
	UINT32 l1d_cachesize, UINT32 l1d_linesize, UINT32 l1d_assoc,
	UINT32 l2_cachesize, UINT32 l2_linesize, UINT32 l2_assoc,
	ApproximateCacheModel L1m, ApproximateCacheModel L2m, ApproximateMemoryModel Mm){
		
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
	
}
