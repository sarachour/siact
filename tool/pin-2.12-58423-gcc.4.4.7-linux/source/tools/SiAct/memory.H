#ifndef MEMORY_H
#define MEMORY_H

#include "pin.H"

class MEMORY_ADDR_RANGES {
	typedef struct {
		UINT64 start;
		UINT64 end;
	} addr_range_t;
	std::vector<addr_range_t> addrs;
	public:
		void def(UINT64 start, UINT32 length); // define a range
		void undef(UINT64 start); // undefine a range
		bool contains(UINT64 addr);
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
