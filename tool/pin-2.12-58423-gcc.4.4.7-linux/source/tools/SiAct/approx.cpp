#include "approx.H"
#include "stdio.h"


template <class SET, UINT32 MAX_SETS, UINT32 STORE_ALLOCATION>
APPROXCACHE<SET,MAX_SETS,STORE_ALLOCATION>::APPROXCACHE(string name, UINT32 cacheSize, UINT32 lineSize, UINT32 associativity, ApproximateCacheModel model):
	CACHE_BASE(name, cacheSize, lineSize, associativity)
{
	
}

template <class SET, UINT32 MAX_SETS, UINT32 STORE_ALLOCATION>
void APPROXCACHE<SET,MAX_SETS,STORE_ALLOCATION>::Description(){
	printf("CACHE %s\n", this->Name().c_str());
	printf("Line Size: %d\n", this->LineSize());
	printf("Cache Size: %d\n", this->CacheSize());
	printf("Associativity: %d\n", this->Associativity());
	switch(model){
		case CacheModelMedium: printf("Model: %s\n", "Medium"); break;
		case CacheModelHeavy: printf("Model: %s\n", "Heavy"); break;
		case CacheModelNone: printf("Model: %s\n", "None"); break;
	}
}

template <class SET, UINT32 MAX_SETS, UINT32 STORE_ALLOCATION>
bool APPROXCACHE<SET,MAX_SETS,STORE_ALLOCATION>::Access(ADDRINT addr, UINT32 size, ACCESS_TYPE accessType){
	const ADDRINT highAddr = addr + size;
    bool allHit = true;

    const ADDRINT lineSize = LineSize();
    const ADDRINT notLineMask = ~(lineSize - 1);
    do
    {
        CACHE_TAG tag;
        UINT32 setIndex;

        SplitAddress(addr, tag, setIndex);

        SET & set = _sets[setIndex];

        bool localHit = set.Find(tag);
        allHit &= localHit;


        // on miss, loads always allocate, stores optionally
        if ( (! localHit) && (accessType == ACCESS_TYPE_LOAD || STORE_ALLOCATION == STORE_ALLOCATE))
        {
            set.Replace(tag);
        }

        addr = (addr & notLineMask) + lineSize; // start of next cache line
    }
    while (addr < highAddr);

    _access[accessType][allHit]++; //hit/miss count of the cache

    return allHit;
}
/// Cache access at addr that does not span cache lines
template <class SET, UINT32 MAX_SETS, UINT32 STORE_ALLOCATION>
bool APPROXCACHE<SET,MAX_SETS,STORE_ALLOCATION>::AccessSingleLine(ADDRINT addr, ACCESS_TYPE accessType){
	CACHE_TAG tag;
    UINT32 setIndex;

    SplitAddress(addr, tag, setIndex);

    SET & set = _sets[setIndex];

    bool hit = set.Find(tag);


    // on miss, loads always allocate, stores optionally
    if ( (! hit) && (accessType == ACCESS_TYPE_LOAD || STORE_ALLOCATION == STORE_ALLOCATE))
    {
        set.Replace(tag);
    }

    _access[accessType][hit]++; //hit/miss count of the cache

    return hit;
}

template <class SET, UINT32 MAX_SETS, UINT32 STORE_ALLOCATION>
void APPROXCACHE<SET,MAX_SETS,STORE_ALLOCATION>::ProcessData(UINT8 * data, ACCESS_TYPE accessType){

}

template <class SET, UINT32 MAX_SETS, UINT32 STORE_ALLOCATION>
void APPROXCACHE<SET,MAX_SETS,STORE_ALLOCATION>::Report(){
	printf("%s\n", this->StatsLong("",CACHE_TYPE_DCACHE).c_str());
}

APPROXMEMORY::APPROXMEMORY(ApproximateMemoryModel model){
	
}
void APPROXMEMORY::Report(){
	
}
void APPROXMEMORY::Description(){
	printf("Approximate Memory\n");
}

template class APPROX_CACHE_LRU(16*KILO, 64, STORE_ALLOCATE);
template class APPROX_CACHE_LRU(64 * KILO, 64, STORE_ALLOCATE);
