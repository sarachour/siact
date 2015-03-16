#include "approx.h"

template <class SET, UINT32 MAX_SETS, UINT32 STORE_ALLOCATION>
bool APPROXCACHE<SET,MAX_SETS,STORE_ALLOCATION>::Access(ADDRINT addr, UINT32 size, ACCESS_TYPE accessType, UINT64* memAccessCount, UINT64* memMissCount)
{
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

        (*memAccessCount)++; //access count of the function

        // on miss, loads always allocate, stores optionally
        if ( (! localHit) && (accessType == ACCESS_TYPE_LOAD || STORE_ALLOCATION == CACHE_ALLOC::STORE_ALLOCATE))
        {
            set.Replace(tag);
            (*memMissCount)++; //miss count of the function
        }

        addr = (addr & notLineMask) + lineSize; // start of next cache line
    }
    while (addr < highAddr);

	//automatically counts approximate accesses
    _access[accessType][allHit]++; //hit/miss count of the cache

    return allHit;
}

/*!
 *  @return true if accessed cache line hits
 */
template <class SET, UINT32 MAX_SETS, UINT32 STORE_ALLOCATION>
bool APPROXCACHE<SET,MAX_SETS,STORE_ALLOCATION>::AccessSingleLine(ADDRINT addr, ACCESS_TYPE accessType, UINT64* memAccessCount, UINT64* memMissCount)
{
    CACHE_TAG tag;
    UINT32 setIndex;

    SplitAddress(addr, tag, setIndex);

    SET & set = _sets[setIndex];

    bool hit = set.Find(tag);

    (*memAccessCount)++; //access count of the function

    // on miss, loads always allocate, stores optionally
    if ( (! hit) && (accessType == ACCESS_TYPE_LOAD || STORE_ALLOCATION == CACHE_ALLOC::STORE_ALLOCATE))
    {
        set.Replace(tag);
        (*memMissCount)++; //miss count of the function
    }

    _access[accessType][hit]++; //hit/miss count of the cache

    return hit;
}
