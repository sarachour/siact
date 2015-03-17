#ifndef APPROX_H
#define APPROX_H

#include "cache.h"
#include "memory.h"
/*
 * APPROXIMATE CACHE THAT TRACKS WHETHER A LINE IS UNRELIABLE OR NOT
 * 
 */
template <class SET, UINT32 MAX_SETS, UINT32 STORE_ALLOCATION>
class APPROXCACHE : public CACHE_BASE{
	private:
		SET _sets[MAX_SETS];
		UINT64 approx_lines;
		UINT64 precise_lines;
		const float PROB_READ_CORRUPT;
		const float PROB_WRITE_CORRUPT;
	public:
	
    // constructors/destructors
    APPROXCACHE(std::string name, 
			UINT32 cacheSize, 
			UINT32 lineSize, 
			UINT32 associativity, 
			float PREAD, 
			float PWRITE);
    // modifiers
    /// Cache access from addr to addr+size-1
    bool Access(ADDRINT addr, UINT32 size, ACCESS_TYPE accessType, UINT64* memAccessCount, UINT64* memMissCount);
    /// Cache access at addr that does not span cache lines
    bool AccessSingleLine(ADDRINT addr, ACCESS_TYPE accessType, UINT64* memAccessCount, UINT64* memMissCount);
    void ReadData(UINT8 * data);
    void Report();
	
};
#define APPROX_CACHE_DIRECT_MAPPED(MAX_SETS, ALLOCATION) APPROXCACHE<DIRECT_MAPPED, MAX_SETS, ALLOCATION>
#define APPROX_CACHE_ROUND_ROBIN(MAX_SETS, MAX_ASSOCIATIVITY, ALLOCATION) APPROXCACHE<ROUND_ROBIN<MAX_ASSOCIATIVITY>, MAX_SETS, ALLOCATION>
#define APPROX_CACHE_LRU(MAX_SETS, MAX_ASSOCIATIVITY, ALLOCATION) APPROXCACHE<LRU<MAX_ASSOCIATIVITY>, MAX_SETS, ALLOCATION>


class APPROXMEMORY : public MEMORY_BASE {
		UINT64 corruptions;
	public:
		void Report();
	
};
#endif
