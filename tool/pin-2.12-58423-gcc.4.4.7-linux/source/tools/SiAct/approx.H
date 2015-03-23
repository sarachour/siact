#ifndef APPROX_H
#define APPROX_H

#include "cache.h"
#include "memory.h"
#include "helper.h"
/*
 * APPROXIMATE CACHE THAT TRACKS WHETHER A LINE IS UNRELIABLE OR NOT
 * 
 */


typedef enum APPROXIMATE_CACHE_MODEL_T {
	CacheModelMedium,
	CacheModelHeavy,
	CacheModelNone
} ApproximateCacheModel;

typedef enum APPROXIMATE_MEMORY_MODEL_T{
	MemoryModelStatic,
	MemoryModelDynamic,
	MemoryModelNone
} ApproximateMemoryModel;


template <class SET, UINT32 MAX_SETS, UINT32 STORE_ALLOCATION>
class APPROXCACHE : public CACHE_BASE{
	private:
		SET _sets[MAX_SETS];
		ApproximateCacheModel model;
		typedef struct APPROXCACHE_STATS {
			UINT64 TOTAL_APPROX_ACCESSES;
			UINT64 TOTAL_PRECISE_ACCESSES;
			UINT64 TOTAL_APPROX_LINES[MAX_SETS];
			UINT64 TOTAL_PRECISE_LINES[MAX_SETS];
			UINT64 N;
		} approx_cache_stats_t;
		
		approx_cache_stats_t acache_stats;
		UINT32 ERR_PROB[3];
		void initApproxStats();
		void updateApproxStats(BOOL approx);
		void printApproxStats(FILE * out);
	public:
		// constructors/destructors
		APPROXCACHE(std::string name, 
				UINT32 cacheSize, 
				UINT32 lineSize, 
				UINT32 associativity, 
				ApproximateCacheModel model);
		// modifiers
		//determine if access should be unreliable
		bool Access(ADDRINT addr, UINT32 size, ACCESS_TYPE accessType, BOOL approx_set, BOOL& approx_get);
		/// Cache access at addr that does not span cache lines
		bool AccessSingleLine(ADDRINT addr, ACCESS_TYPE accessType, BOOL approx_set, BOOL& approx_get);
		bool ProcessData(UINT8 * data, UINT32 size, ACCESS_TYPE accessType);
		void Report(FILE * out);
		void Description(FILE * out);
};
#define APPROX_CACHE_LRU(MAX_SETS, MAX_ASSOCIATIVITY, ALLOCATION) APPROXCACHE<ALRU<MAX_ASSOCIATIVITY>, MAX_SETS, ALLOCATION>


class APPROXMEMORY : public MEMORY_BASE {
		typedef struct MEM_STATS{
			UINT64 NREADS;
			UINT64 NWRITES;
			UINT64 NCORRUPTIONS;
		} memory_stats_t;
		//2*4 Byte Combo Map - neighbors on row-row and word-word
		float spatial[8];
		UINT32 masks[8];
		memory_stats_t stats;
		MEMORY_ADDR_RANGES * regions;
		ApproximateMemoryModel model;
		void initMemoryStats();
		void updateMemoryStatsReads(bool isread);
		void updateMemoryStatsCorruption();
		void printMemoryStats(FILE * out);
	public:
		APPROXMEMORY(ApproximateMemoryModel model, MEMORY_ADDR_RANGES * regions);
		bool ProcessData(ADDRINT addr, UINT8 * data, UINT32 size, ACCESS_TYPE accessType);
		void Report(FILE * out);
		void Description(FILE * out);
		void Refresh();
		void Accumulate(float msec);
	
};
#endif
