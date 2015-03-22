#ifndef MEMORY_H
#define MEMORY_H

#include "pin.H"

class MEMORY_ADDR_RANGES {
	typedef struct {
		UINT64 N_REFRESHES;
		UINT64 N_FORCE_REFRESHES;
		float DRAM_LATENCY_SUM;
		float MAX_DRAM_LATENCY;
		float MIN_DRAM_LATENCY;
	} region_stats_t;
	
	region_stats_t stats;
	
	typedef struct {
		UINT64 start; //start of range
		UINT64 end; //end of range
		UINT64 msec; //time since last refresh
	} addr_range_t;
	std::vector<addr_range_t> addrs;
		UINT32 find(UINT64 addr);
	void init_stats();
	void update_stats(float latency);
	void print_stats(FILE * out);
	public:
		MEMORY_ADDR_RANGES();
		void def(UINT64 start, UINT32 length); // define a range
		void undef(UINT64 start); // undefine a range
		bool contains(UINT64 addr);
		void refresh_all();
		void refresh(UINT64 addr); //refresh address
		void accumulate(UINT64 msec); //accumulate all ranges
		UINT64 elapsed(UINT64 addr);
		void report(FILE * out);
};

class MEMORY_BASE {
		
	public:
		UINT64 Reads();
		UINT64 Writes();
};

class MEMORY : public MEMORY_BASE {
	public:
		void Report(FILE * out);
		void Description(FILE * out);
};
extern "C" {
	VOID mem_urel_alloc(UINT64 address, UINT32 length);
}
#endif
