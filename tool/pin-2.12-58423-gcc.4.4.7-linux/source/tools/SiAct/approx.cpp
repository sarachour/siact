#include "approx.H"
#include "stdio.h"
#include "helper.H"
#include "math.h"
#include "xorshift.H"
#include "pin.H"

template <class SET, UINT32 MAX_SETS, UINT32 STORE_ALLOCATION>
APPROXCACHE<SET,MAX_SETS,STORE_ALLOCATION>::APPROXCACHE(string name, UINT32 cacheSize, UINT32 lineSize, UINT32 associativity, ApproximateCacheModel model):
	CACHE_BASE(name, cacheSize, lineSize, associativity)
{
	ASSERTX(NumSets() <= MAX_SETS);

	
	for (UINT32 i = 0; i < NumSets(); i++)
	{
		_sets[i].SetAssociativity(associativity);
	}
	this->model = model;
	ERR_PROB[ACCESS_TYPE_NUM] = 0;
	switch(this->model){
		case CacheModelMedium:
			ERR_PROB[ACCESS_TYPE_LOAD]  = pow(10,-7.4)*RGEN_MAX;
			ERR_PROB[ACCESS_TYPE_STORE]  = pow(10,-4.94)*RGEN_MAX;
			break;
		case CacheModelHeavy:
			ERR_PROB[ACCESS_TYPE_LOAD]  = pow(10,-3)*RGEN_MAX;
			ERR_PROB[ACCESS_TYPE_STORE]  = pow(10,-3)*RGEN_MAX;
			break;
		case CacheModelNone:
			ERR_PROB[ACCESS_TYPE_LOAD]  = 0;
			ERR_PROB[ACCESS_TYPE_STORE] = 0;
			break;
	}
	initApproxStats();
}

template <class SET, UINT32 MAX_SETS, UINT32 STORE_ALLOCATION>
void APPROXCACHE<SET,MAX_SETS,STORE_ALLOCATION>::initApproxStats(){
	acache_stats.TOTAL_APPROX_ACCESSES = 0;
	acache_stats.TOTAL_PRECISE_ACCESSES = 0;
	for(int i=0; i < MAX_SETS; i++){
		acache_stats.TOTAL_APPROX_LINES[i] = 0;
		acache_stats.TOTAL_PRECISE_LINES[i] = 0;
	}
	acache_stats.N = 0;
}

template <class SET, UINT32 MAX_SETS, UINT32 STORE_ALLOCATION>
void APPROXCACHE<SET,MAX_SETS,STORE_ALLOCATION>::updateApproxStats(BOOL isapprox){
	if(isapprox) acache_stats.TOTAL_APPROX_ACCESSES += 1;
	else acache_stats.TOTAL_PRECISE_ACCESSES += 1;
	for(int i=0; i < NumSets(); i++){
		acache_stats.TOTAL_APPROX_LINES[i] += _sets[i].getNApproxLines();
		acache_stats.TOTAL_PRECISE_LINES[i] += _sets[i].getNPreciseLines();
	}
	acache_stats.N++;
}

template <class SET, UINT32 MAX_SETS, UINT32 STORE_ALLOCATION>
void APPROXCACHE<SET,MAX_SETS,STORE_ALLOCATION>::printApproxStats(FILE * out){
	float frac_approx = static_cast<float>(acache_stats.TOTAL_APPROX_ACCESSES)/(acache_stats.TOTAL_APPROX_ACCESSES+acache_stats.TOTAL_PRECISE_ACCESSES);
	fprintf(out,"Approximate Accesses: %ld\n", acache_stats.TOTAL_APPROX_ACCESSES);
	fprintf(out,"Precise Accesses: %ld\n", acache_stats.TOTAL_PRECISE_ACCESSES);
	fprintf(out,"Percent Approximate Accesses: %f%%\n", frac_approx*100.0);
	float TOTAL_APPROX_LINES = 0;
	float TOTAL_PRECISE_LINES = 0;
	for(int i=0; i < NumSets(); i++){
		TOTAL_APPROX_LINES += static_cast<float>(acache_stats.TOTAL_APPROX_LINES[i]);
		TOTAL_PRECISE_LINES += static_cast<float>(acache_stats.TOTAL_PRECISE_LINES[i]);
	}
	frac_approx = TOTAL_APPROX_LINES/(TOTAL_APPROX_LINES + TOTAL_PRECISE_LINES);
	TOTAL_APPROX_LINES /= static_cast<float>(acache_stats.N);
	TOTAL_PRECISE_LINES /= static_cast<float>(acache_stats.N);
	fprintf(out,"Percent Approximate Lines: %f%%\n", frac_approx*100.0);
}

template <class SET, UINT32 MAX_SETS, UINT32 STORE_ALLOCATION>
void APPROXCACHE<SET,MAX_SETS,STORE_ALLOCATION>::Description(FILE * out){
	fprintf(out,"CACHE %s\n", this->Name().c_str());
	fprintf(out,"Line Size: %d\n", this->LineSize());
	fprintf(out,"Cache Size: %d\n", this->CacheSize());
	fprintf(out,"Associativity: %d\n", this->Associativity());
	switch(model){
		case CacheModelMedium: fprintf(out,"Model: %s\n", "Medium"); break;
		case CacheModelHeavy: fprintf(out,"Model: %s\n", "Heavy"); break;
		case CacheModelNone: fprintf(out,"Model: %s\n", "None"); break;
	}
	fprintf(out,"Perr(READ) = %e\n", static_cast<float>(ERR_PROB[ACCESS_TYPE_LOAD])/RGEN_MAX );
	fprintf(out,"Perr(WRITE) = %e\n", static_cast<float>(ERR_PROB[ACCESS_TYPE_STORE])/RGEN_MAX );
}

template <class SET, UINT32 MAX_SETS, UINT32 STORE_ALLOCATION>
bool APPROXCACHE<SET,MAX_SETS,STORE_ALLOCATION>::Access(ADDRINT addr, UINT32 size, ACCESS_TYPE accessType, BOOL SET_APPROX, BOOL& GET_APPROX){
	const ADDRINT highAddr = addr + size;
	const ADDRINT lowAddr = addr;
    bool allHit = true;

    const ADDRINT lineSize = LineSize();
    const ADDRINT notLineMask = ~(lineSize - 1);
    do
    {
        CACHE_TAG tag;
        UINT32 setIndex;

        SplitAddress(addr, tag, setIndex);

        SET & set = _sets[setIndex];

        bool localHit = set.Find(tag, GET_APPROX);
        allHit &= localHit;


        // on miss, loads always allocate, stores optionally
        if ( (! localHit) && (accessType == ACCESS_TYPE_LOAD || STORE_ALLOCATION == STORE_ALLOCATE))
        {
            set.Replace(tag, SET_APPROX);
			updateApproxStats(SET_APPROX);
        }
        else if(localHit && !GET_APPROX && SET_APPROX){
            set.setApprox(tag, SET_APPROX);
			updateApproxStats(SET_APPROX);
		}
		else if(localHit && GET_APPROX && !SET_APPROX){
            set.setApprox(tag, SET_APPROX);
			updateApproxStats(SET_APPROX);
		}
        else{
			updateApproxStats(GET_APPROX);
		}
        addr = (addr & notLineMask) + lineSize; // start of next cache line
    }
    while (addr < highAddr && addr >= lowAddr);

    _access[accessType][allHit]++; //hit/miss count of the cache
    return allHit;
}
/// Cache access at addr that does not span cache lines
template <class SET, UINT32 MAX_SETS, UINT32 STORE_ALLOCATION>
bool APPROXCACHE<SET,MAX_SETS,STORE_ALLOCATION>::AccessSingleLine(ADDRINT addr, ACCESS_TYPE accessType, BOOL SET_APPROX, BOOL& GET_APPROX){
	CACHE_TAG tag;
    UINT32 setIndex;

    SplitAddress(addr, tag, setIndex);

    SET & set = _sets[setIndex];

    bool hit = set.Find(tag, GET_APPROX);


    // on miss, loads always allocate, stores optionally
    if ( (! hit) && (accessType == ACCESS_TYPE_LOAD || STORE_ALLOCATION == STORE_ALLOCATE))
    {
        set.Replace(tag, SET_APPROX);
        updateApproxStats(SET_APPROX);
    }
    else{
		updateApproxStats(GET_APPROX);
	}

    _access[accessType][hit]++; //hit/miss count of the cache

    return hit;
}

template <class SET, UINT32 MAX_SETS, UINT32 STORE_ALLOCATION>
void APPROXCACHE<SET,MAX_SETS,STORE_ALLOCATION>::ProcessData(UINT8 * data, UINT32 size, ACCESS_TYPE accessType){
	UINT32 PROB = ERR_PROB[accessType];
	if(xorshift32() < PROB){
			printf("corrupt %d ", data[0]);
			UINT64 mask = xorshift64();
			PIN_SafeCopy(data, &mask, size);
			printf("-> %d\n", data[0]);
	}
}

template <class SET, UINT32 MAX_SETS, UINT32 STORE_ALLOCATION>
void APPROXCACHE<SET,MAX_SETS,STORE_ALLOCATION>::Report(FILE * out){
	fprintf(out,"%s\n", this->StatsLong("",CACHE_TYPE_DCACHE).c_str());
	printApproxStats(out);
}

APPROXMEMORY::APPROXMEMORY(ApproximateMemoryModel model){
	
}
void APPROXMEMORY::Report(FILE * out){
	fprintf(out,"Approximate Memory\n");
}
void APPROXMEMORY::Description(FILE * out){
	fprintf(out,"Approximate Memory\n");
}

template class APPROX_CACHE_LRU(16*KILO, 64, STORE_ALLOCATE);
template class APPROX_CACHE_LRU(64 * KILO, 64, STORE_ALLOCATE);
