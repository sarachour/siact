#ifndef APPROX_H
#define APPROX_H

#include "cache.h"
#include "memory.h"
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
	public:
		// constructors/destructors
		APPROXCACHE(std::string name, 
				UINT32 cacheSize, 
				UINT32 lineSize, 
				UINT32 associativity, 
				ApproximateCacheModel model);
		// modifiers
		//determine if access should be unreliable
		bool Access(ADDRINT addr, UINT32 size, ACCESS_TYPE accessType);
		/// Cache access at addr that does not span cache lines
		bool AccessSingleLine(ADDRINT addr, ACCESS_TYPE accessType);
		void ReadData(UINT8 * data);
		void Report();
		void Description();
};
#define APPROX_CACHE_DIRECT_MAPPED(MAX_SETS, ALLOCATION) APPROXCACHE<DIRECT_MAPPED, MAX_SETS, ALLOCATION>
#define APPROX_CACHE_ROUND_ROBIN(MAX_SETS, MAX_ASSOCIATIVITY, ALLOCATION) APPROXCACHE<ROUND_ROBIN<MAX_ASSOCIATIVITY>, MAX_SETS, ALLOCATION>
#define APPROX_CACHE_LRU(MAX_SETS, MAX_ASSOCIATIVITY, ALLOCATION) APPROXCACHE<LRU<MAX_ASSOCIATIVITY>, MAX_SETS, ALLOCATION>


class APPROXMEMORY : public MEMORY_BASE {
		UINT64 corruptions;
		
		/*
		 * 
		 *INSTLIB::ALARM_ICOUNT DRAM_ALARM;
		#define INSTRUCTIONS_PER_MILLISECOND 2000*1000
		VOID CORRUPT_DRAM_CALLBACK(VOID * val, CONTEXT * ctxt, VOID * ip, THREADID tid){
			INSTLIB::ALARM_ICOUNT * al = static_cast<INSTLIB::ALARM_ICOUNT*>(val);
			report_corrupted_dram();
			al->SetAlarm(INSTRUCTIONS_PER_MILLISECOND, CORRUPT_DRAM_CALLBACK, &DRAM_ALARM);
} 
		*/
			/*
		DRAM_ALARM.Activate();
		DRAM_ALARM.SetAlarm(INSTRUCTIONS_PER_MILLISECOND, CORRUPT_DRAM_CALLBACK, &DRAM_ALARM);
	*/
	public:
		APPROXMEMORY(ApproximateMemoryModel model);
		void Report();
		void Description();
	
};
#endif
