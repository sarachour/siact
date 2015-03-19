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
		void printApproxStats();
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
		void ProcessData(UINT8 * data, UINT32 size, ACCESS_TYPE accessType);
		void Report();
		void Description();
};
#define APPROX_CACHE_LRU(MAX_SETS, MAX_ASSOCIATIVITY, ALLOCATION) APPROXCACHE<ALRU<MAX_ASSOCIATIVITY>, MAX_SETS, ALLOCATION>


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
