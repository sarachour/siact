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
	L1D = new CACHE_LRU(16 * KILO, 64, STORE_ALLOCATE)("L1D",l1d_cachesize, l1d_linesize, l1d_assoc);
	L2 = new CACHE_LRU(64 * KILO, 64, STORE_ALLOCATE)("L2",l2_cachesize, l2_linesize, l2_assoc);
	
	MEM = new MEMORY();
	
}
PreciseHierarchy::~PreciseHierarchy(){
	delete L1I;
	delete L1D;
	delete L2;
	delete MEM;
	
}
void PreciseHierarchy::load(ADDRINT addr, UINT8 * data, UINT32 size){
	
}
void PreciseHierarchy::store(ADDRINT addr, UINT8 * data, UINT32 size){
	
}
void PreciseHierarchy::report(FILE * out){
	fprintf(out,"#### REPORT ######\n");
	fprintf(out,"Precise Hierarchy\n");
	fprintf(out,"===== Caches =====\n");
	L1I->Report(out);
	fprintf(out,"-----------------\n");
	L1D->Report(out);
	fprintf(out,"-----------------\n");
	L2->Report(out);
	fprintf(out,"-----------------\n");
	fprintf(out,"===== Main Memory =====\n");
	MEM->Report(out);
	fprintf(out,"-----------------\n");
}

void PreciseHierarchy::description(FILE * out){
	fprintf(out,"#### DESCRIPTION ######\n");
	printf("Precise Hierarchy\n");
	fprintf(out,"===== Caches =====\n");
	L1I->Description(out);
	fprintf(out,"-----------------\n");
	L1D->Description(out);
	fprintf(out,"-----------------\n");
	L2->Description(out);
	fprintf(out,"-----------------\n");
	fprintf(out,"===== Main Memory =====\n");
	MEM->Description(out);
	fprintf(out,"-----------------\n");
}


ApproximateHierarchy::ApproximateHierarchy(UINT32 l1i_cachesize, UINT32 l1i_linesize, UINT32 l1i_assoc,
	UINT32 l1d_cachesize, UINT32 l1d_linesize, UINT32 l1d_assoc,
	UINT32 l2_cachesize, UINT32 l2_linesize, UINT32 l2_assoc,
	ApproximateCacheModel L1m, ApproximateCacheModel L2m, ApproximateMemoryModel Mm){
	L1I = new CACHE_LRU(16 * KILO, 64, STORE_ALLOCATE)("L1I",l1i_cachesize, l1i_linesize, l1i_assoc);
	L1D = new APPROX_CACHE_LRU(16 * KILO, 64, STORE_ALLOCATE)("L1D",l1d_cachesize, l1d_linesize, l1d_assoc, L1m);
	L2 = new APPROX_CACHE_LRU(64 * KILO, 64, STORE_ALLOCATE)("L2",l2_cachesize, l2_linesize, l2_assoc, L2m);
	MEM = new APPROXMEMORY(Mm, &RANGES);
}
void ApproximateHierarchy::load(ADDRINT addr, UINT8 * data, UINT32 size, BOOL approx, BOOL& is_transient_error){
	bool APPROX_SET = approx;
	bool APPROX_GET;
	bool L1_HIT = L1D->Access(addr, size,ACCESS_TYPE_LOAD, APPROX_SET, APPROX_GET);
	is_transient_error = true;
	if(L1_HIT && APPROX_GET){
		L1D->ProcessData(data, size, ACCESS_TYPE_LOAD);
	}
	else if(!L1_HIT){
		bool L2_HIT = L2->Access(addr,size,ACCESS_TYPE_LOAD, APPROX_SET, APPROX_GET);
		if(L2_HIT && APPROX_GET){
			L2->ProcessData(data, size, ACCESS_TYPE_LOAD);
		}
		else if(!L2_HIT){
			MEM->ProcessData(addr,data,size,ACCESS_TYPE_LOAD);
		}
	}
}

void ApproximateHierarchy::store(ADDRINT addr, UINT8 * data, UINT32 size, BOOL approx){
	bool APPROX_SET = approx;
	bool APPROX_GET;
	
	bool L1_HIT = L1D->Access(addr, size,ACCESS_TYPE_STORE, APPROX_SET, APPROX_GET);
	if(L1_HIT && APPROX_GET){
		L1D->ProcessData(data, size, ACCESS_TYPE_STORE);
	}
	else if(!L1_HIT){
		bool L2_HIT = L2->Access(addr, size, ACCESS_TYPE_STORE, APPROX_SET, APPROX_GET);
		if(L2_HIT && APPROX_GET){
			L2->ProcessData(data, size, ACCESS_TYPE_STORE);
		}
		else if(!L2_HIT){
			MEM->ProcessData(addr,data,size,ACCESS_TYPE_STORE);
			//TODO
		}
	}
}
void ApproximateHierarchy::refresh(){
	MEM->Refresh();
}
void ApproximateHierarchy::alloc(ADDRINT data, UINT32 size, BOOL approx){
	if(approx) RANGES.def(data, size);
	else RANGES.undef(data);
}
bool ApproximateHierarchy::approx(ADDRINT data){
	return RANGES.contains(data);
}
void ApproximateHierarchy::report(FILE * out){
	fprintf(out,"#### REPORT ######\n");
	fprintf(out,"Approximate Hierarchy\n");
	fprintf(out,"===== Caches =====\n");
	L1I->Report(out);
	fprintf(out,"-----------------\n");
	L1D->Report(out);
	fprintf(out,"-----------------\n");
	L2->Report(out);
	fprintf(out,"-----------------\n");
	fprintf(out,"===== Main Memory =====\n");
	MEM->Report(out);
	fprintf(out,"-----------------\n");
}
void ApproximateHierarchy::elapsed(UINT64 msec){
	MEM->Accumulate(msec);
}
void ApproximateHierarchy::description(FILE * out){
	fprintf(out,"#### DESCRIPTION ######\n");
	fprintf(out,"Approximate Hierarchy\n");
	fprintf(out,"===== Caches =====\n");
	L1I->Description(out);
	fprintf(out,"-----------------\n");
	L1D->Description(out);
	fprintf(out,"-----------------\n");
	L2->Description(out);
	fprintf(out,"-----------------\n");
	fprintf(out,"===== Main Memory =====\n");
	MEM->Description(out);
	fprintf(out,"-----------------\n");
}
ApproximateHierarchy::~ApproximateHierarchy(){
	delete L1I;
	delete L1D;
	delete L2;
	delete MEM;
	
}
