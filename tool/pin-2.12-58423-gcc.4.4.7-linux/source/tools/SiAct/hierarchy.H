#ifndef CACHE_HIERARCHY
#define CACHE_HIERARCHY

#include "cache.H"
#include "approx.H"
#include "pin.H"


class PreciseHierarchy {
		//l1 cache
		CACHE_LRU(16 * KILO, 64, STORE_ALLOCATE) * L1I;
		CACHE_LRU(16 * KILO, 64, STORE_ALLOCATE) * L1D;
		//l2 cache
		CACHE_LRU(64 * KILO, 64, STORE_ALLOCATE) * L2;
		MEMORY * MEM;
	public:
		 PreciseHierarchy(UINT32 l1i_cachesize, UINT32 l1i_linesize, UINT32 l1i_assoc,
			UINT32 l1d_cachesize, UINT32 l1d_linesize, UINT32 l1d_assoc,
			UINT32 l2_cachesize, UINT32 l2_linesize, UINT32 l2_assoc);
		 ~PreciseHierarchy();
		 void load(ADDRINT addr, UINT8 * data, UINT32 size);
		 void store(ADDRINT addr, UINT8 * data, UINT32 size);
		 void report(FILE * out);
		 void description(FILE * out);
};

class ApproximateHierarchy {
		CACHE_LRU(16 * KILO, 64, STORE_ALLOCATE) * L1I;
		APPROX_CACHE_LRU(16 * KILO, 64, STORE_ALLOCATE) * L1D;
		APPROX_CACHE_LRU(64 * KILO, 64, STORE_ALLOCATE) * L2;
		APPROXMEMORY * MEM;
		MEMORY_ADDR_RANGES RANGES;
	public:
		 ApproximateHierarchy(UINT32 l1i_cachesize, UINT32 l1i_linesize, UINT32 l1i_assoc,
			UINT32 l1d_cachesize, UINT32 l1d_linesize, UINT32 l1d_assoc,
			UINT32 l2_cachesize, UINT32 l2_linesize, UINT32 l2_assoc,
			ApproximateCacheModel L1m, ApproximateCacheModel L2m, ApproximateMemoryModel Mm);
		~ApproximateHierarchy();
		 void load(ADDRINT addr, UINT8 * data, UINT32 size, BOOL approx, BOOL& is_transient_error);
		 void store(ADDRINT addr, UINT8 * data, UINT32 size, BOOL approx);
		 void alloc(ADDRINT data, UINT32 size, BOOL approx);
		 bool approx(ADDRINT data);
		 void report(FILE * out);
		 void elapsed(UINT64 msec);
		 void refresh();
		 void description(FILE * out);
};

template <class S, class T> 
class Tuple {
	S * one;
	T * two;
	public:
		Tuple(S * in){one = in; two=0;}
		Tuple(T * in){two = in; one=0;}
		bool hasFirst(){return (one != 0);}
		bool hasSecond(){return (two != 0);}
		S * first(){return one;}
		T * second(){return two;}
	
};

extern Tuple<PreciseHierarchy,ApproximateHierarchy> * memory;
#endif
