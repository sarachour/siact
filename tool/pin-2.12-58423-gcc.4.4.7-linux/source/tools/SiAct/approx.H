#ifndef APPROX_H
#define APPROX_H

#include "cache.h"
#include "memory.h"
/*
 * APPROXIMATE CACHE THAT TRACKS WHETHER A LINE IS UNRELIABLE OR NOT
 * 
 */
extern "C" {

	typedef enum {
		CacheModelMedium,
		CacheModelHeavy,
		CacheModelNone
	} ApproximateCacheModel;

	typedef enum {
		MemoryModelStatic,
		MemoryModelDynamic,
		MemoryModelNone
	} ApproximateMemoryModel;

}
template <class SET, UINT32 MAX_SETS, UINT32 STORE_ALLOCATION>
class APPROXCACHE : public CACHE_BASE{
	private:
		SET _sets[MAX_SETS];
		UINT64 approx_lines;
		UINT64 precise_lines;
		const float PROB_READ_CORRUPT;
		const float PROB_WRITE_CORRUPT;
		ApproximateCacheModel model;
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
		void SetModel(ApproximateCacheModel c);
		bool Access(ADDRINT addr, UINT32 size, ACCESS_TYPE accessType);
		/// Cache access at addr that does not span cache lines
		bool AccessSingleLine(ADDRINT addr, ACCESS_TYPE accessType);
		void ReadData(UINT8 * data);
		void Report();
	
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
		void setModel(ApproximateMemoryModel model);
		void Report();
	
};
#endif
