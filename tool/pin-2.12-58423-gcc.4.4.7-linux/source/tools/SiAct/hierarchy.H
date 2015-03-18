#ifndef CACHE_HIERARCHY
#define CACHE_HIERARCHY

#include "cache.H"
#include "approx.H"

class CacheHierarchy {
	public:
		virtual VOID load(UINT64 addr, UINT8 data) = 0;
		virtual VOID store(UINT64 addr, UINT8 data) = 0;
		virtual VOID report() = 0;
		virtual VOID elapsed(UINT64 msec) = 0;
		virtual VOID description() = 0;
		
		
};

class PreciseHierarchy : public CacheHierarchy{
		//l1 cache
		CACHE_LRU(16 * KILO, 64, STORE_ALLOCATE) * L1I;
		CACHE_LRU(16 * KILO, 64, STORE_ALLOCATE) * L1D;
		//l2 cache
		CACHE_LRU(64 * KILO, 64, STORE_ALLOCATE) * L2;
		MEMORY MEM;
	public:
		 PreciseHierarchy(UINT32 l1i_cachesize, UINT32 l1i_linesize, UINT32 l1i_assoc,
			UINT32 l1d_cachesize, UINT32 l1d_linesize, UINT32 l1d_assoc,
			UINT32 l2_cachesize, UINT32 l2_linesize, UINT32 l2_assoc);
		 void load(UINT64 addr, UINT8 data);
		 void store(UINT64 addr, UINT8 data);
		 void report();
		 void elapsed(UINT64 msec);
		 void description();
};

class ApproximateHierarchy : public CacheHierarchy {
		CACHE_LRU(16 * KILO, 64, STORE_ALLOCATE) * L1I;
		APPROX_CACHE_LRU(16 * KILO, 64, STORE_ALLOCATE) * L1D;
		APPROX_CACHE_LRU(64 * KILO, 64, STORE_ALLOCATE) * L2;
		APPROXMEMORY MEM;
		MEMORY_ADDR_RANGES RANGES;
	public:
		 ApproximateHierarchy(UINT32 l1i_cachesize, UINT32 l1i_linesize, UINT32 l1i_assoc,
			UINT32 l1d_cachesize, UINT32 l1d_linesize, UINT32 l1d_assoc,
			UINT32 l2_cachesize, UINT32 l2_linesize, UINT32 l2_assoc,
			ApproximateCacheModel L1m, ApproximateCacheModel L2m, ApproximateMemoryModel Mm);
		 void load(UINT64 addr, UINT8 data);
		 void store(UINT64 addr, UINT8 data);
		 void report();
		 void elapsed(UINT64 msec);
		 void description();
};


#endif
