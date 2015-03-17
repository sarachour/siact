#ifndef MEMORY_H
#define MEMORY_H

class MEMORY_ADDR_RANGES {
	typedef struct {
		UINT64 start;
		UINT64 end;
	} addr_range_t;
	std::vector<addr_range_t> addrs;
	public:
		VOID def(UINT64 start, UINT64 length); // define a range
		VOID undef(UINT64 start); // undefine a range
		VOID contains(UINT64 addr);
};

class MEMORY_BASE {
	public:
		UINT64 Reads();
		UINT64 Writes();
		
	
};

class MEMORY : public MEMORY_BASE {
	
};



#endif
