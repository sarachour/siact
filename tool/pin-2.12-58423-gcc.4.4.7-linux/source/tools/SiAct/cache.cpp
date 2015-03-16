#include "cache.h"

#include <sstream>
#include <algorithm>
#include <vector>
#include <map>
#include <assert.h>

/*!
 *  @brief Cache tag - self clearing on creation
 */

//CACHE_TAG(ADDRINT tag = 0) { _tag = tag; }
CACHE_TAG::CACHE_TAG(ADDRINT tag = 0, string cacheLineOwner = "DEFAULT") { 
	_tag = tag; 
	_cacheLineOwner = cacheLineOwner; 
}
bool CACHE_TAG::operator==(const CACHE_TAG &right) const { 
	return _tag == right._tag; 
}
operator CACHE_TAG::ADDRINT() const { 
	return _tag; 
}
void CACHE_TAG::setCacheLineOwner(string cacheLineOwner){ 
	_cacheLineOwner = cacheLineOwner;
}
string CACHE_TAG::updateCacheLineOwner(string cacheLineOwner){ 
	string prevCacheOwner = _cacheLineOwner; 
	_cacheLineOwner = cacheLineOwner; 
	return(prevCacheOwner);
}
string CACHE_TAG::getCacheLineOwner(){ 
	return(_cacheLineOwner);
}


CACHE_STATS CACHE_BASE::SumAccess(bool hit) const
{
	CACHE_STATS sum = 0;

	for (UINT32 accessType = 0; accessType < ACCESS_TYPE_NUM; accessType++)
	{
		sum += _access[accessType][hit];
	}

	return sum;
}

CACHE_BASE::CACHE_BASE(std::string name, UINT32 cacheSize, UINT32 lineSize, UINT32 associativity)
  : _name(name),
    _cacheSize(cacheSize),
    _lineSize(lineSize),
    _associativity(associativity),
    _lineShift(FloorLog2(lineSize)),
    _setIndexMask((cacheSize / (associativity * lineSize)) - 1)
{

    ASSERTX(IsPower2(_lineSize));
    ASSERTX(IsPower2(_setIndexMask + 1));

    for (UINT32 accessType = 0; accessType < ACCESS_TYPE_NUM; accessType++)
    {
        _access[accessType][false] = 0;
        _access[accessType][true] = 0;
    }
}

UINT32 CACHE_BASE::NumSets() const { return _setIndexMask + 1; }
VOID CACHE_BASE::SplitAddress(const ADDRINT addr, CACHE_TAG & tag, UINT32 & setIndex) const
{
	tag = addr >> _lineShift;
	setIndex = tag & _setIndexMask;
}

VOID CACHE_BASE::SplitAddress(const ADDRINT addr, CACHE_TAG & tag, UINT32 & setIndex, UINT32 & lineIndex) const
{
	const UINT32 lineMask = _lineSize - 1;
	lineIndex = addr & lineMask;
	SplitAddress(addr, tag, setIndex);
}


// constructors/destructors
CACHE::CACHE(std::string name, UINT32 cacheSize, UINT32 lineSize, UINT32 associativity)
  : CACHE_BASE(name, cacheSize, lineSize, associativity)
{
	ASSERTX(NumSets() <= MAX_SETS);

	for (UINT32 i = 0; i < NumSets(); i++)
	{
		_sets[i].SetAssociativity(associativity);
	}
}
/*!
 *  @brief Stats output method
 */

string CACHE_BASE::StatsLong(string prefix, CACHE_TYPE cache_type) const
{
    const UINT32 headerWidth = 19;
    const UINT32 numberWidth = 12;

    string out;
    
    out += prefix + _name + ":" + "\n";

    if (cache_type != CACHE_TYPE_ICACHE) {
       for (UINT32 i = 0; i < ACCESS_TYPE_NUM; i++)
       {
           const ACCESS_TYPE accessType = ACCESS_TYPE(i);

           std::string type(accessType == ACCESS_TYPE_LOAD ? "Load" : "Store");

           out += prefix + ljstr(type + "-Hits:      ", headerWidth)
                  + mydecstr(Hits(accessType), numberWidth)  +
                  "  " +fltstr(100.0 * Hits(accessType) / Accesses(accessType), 2, 6) + "%\n";

           out += prefix + ljstr(type + "-Misses:    ", headerWidth)
                  + mydecstr(Misses(accessType), numberWidth) +
                  "  " +fltstr(100.0 * Misses(accessType) / Accesses(accessType), 2, 6) + "%\n";
        
           out += prefix + ljstr(type + "-Accesses:  ", headerWidth)
                  + mydecstr(Accesses(accessType), numberWidth) +
                  "  " +fltstr(100.0 * Accesses(accessType) / Accesses(accessType), 2, 6) + "%\n";
        
           out += prefix + "\n";
       }
    }

    out += prefix + ljstr("Total-Hits:      ", headerWidth)
           + mydecstr(Hits(), numberWidth) +
           "  " +fltstr(100.0 * Hits() / Accesses(), 2, 6) + "%\n";

    out += prefix + ljstr("Total-Misses:    ", headerWidth)
           + mydecstr(Misses(), numberWidth) +
           "  " +fltstr(100.0 * Misses() / Accesses(), 2, 6) + "%\n";

    out += prefix + ljstr("Total-Accesses:  ", headerWidth)
           + mydecstr(Accesses(), numberWidth) +
           "  " +fltstr(100.0 * Accesses() / Accesses(), 2, 6) + "%\n";
    out += "\n";

    return out;
}


/*!
 *  @return true if all accessed cache lines hit
 */

template <class SET, UINT32 MAX_SETS, UINT32 STORE_ALLOCATION>
bool CACHE<SET,MAX_SETS,STORE_ALLOCATION>::Access(ADDRINT addr, UINT32 size, ACCESS_TYPE accessType, UINT64* memAccessCount, UINT64* memMissCount)
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

    _access[accessType][allHit]++; //hit/miss count of the cache

    return allHit;
}

/*!
 *  @return true if accessed cache line hits
 */
template <class SET, UINT32 MAX_SETS, UINT32 STORE_ALLOCATION>
bool CACHE<SET,MAX_SETS,STORE_ALLOCATION>::AccessSingleLine(ADDRINT addr, ACCESS_TYPE accessType, UINT64* memAccessCount, UINT64* memMissCount)
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

namespace DL1
{
    
    const UINT32 max_sets = 16 * KILO; // cacheSize / (lineSize * associativity);
    const UINT32 max_associativity = 64; // associativity;
    const CACHE_ALLOC::STORE_ALLOCATION allocation = CACHE_ALLOC::STORE_ALLOCATE;
    //typedef CACHE_ROUND_ROBIN(max_sets, max_associativity, allocation) CACHE;
    typedef CACHE_LRU(max_sets, max_associativity, allocation) CACHE;

}

DL1::CACHE* dl1 = NULL;

namespace D_AxC
{

    const UINT32 max_sets = 16 * KILO; // cacheSize / (lineSize * associativity);
    const UINT32 max_associativity = 64; // associativity;
    const CACHE_ALLOC::STORE_ALLOCATION allocation = CACHE_ALLOC::STORE_ALLOCATE;
    //typedef CACHE_ROUND_ROBIN(max_sets, max_associativity, allocation) CACHE;
    typedef CACHE_LRU(max_sets, max_associativity, allocation) CACHE;

}

D_AxC::CACHE* d_axc = NULL;

namespace UL2
{

    const UINT32 max_sets = 64 * KILO; // cacheSize / (lineSize * associativity);
    const UINT32 max_associativity = 64; // associativity;
    const CACHE_ALLOC::STORE_ALLOCATION allocation = CACHE_ALLOC::STORE_ALLOCATE;
    //typedef CACHE_ROUND_ROBIN(max_sets, max_associativity, allocation) CACHE;
    typedef CACHE_LRU(max_sets, max_associativity, allocation) CACHE;

}
UL2::CACHE* ul2 = NULL;

VOID LoadMultiFast(ADDRINT addr, UINT32 size, UINT64* L1AccessCount, UINT64* L1MissCount, UINT64* L2AccessCount, UINT64* L2MissCount)
{

    if( dl1->Access(addr, size, CACHE_BASE::ACCESS_TYPE_LOAD, L1AccessCount, L1MissCount) == false )
    {
        ul2->Access(addr, size, CACHE_BASE::ACCESS_TYPE_LOAD, L2AccessCount, L2MissCount);
    }

}

/* ===================================================================== */

VOID StoreMultiFast(ADDRINT addr, UINT32 size, UINT64* L1AccessCount, UINT64* L1MissCount, UINT64* L2AccessCount, UINT64* L2MissCount)
{

    if( dl1->Access(addr, size, CACHE_BASE::ACCESS_TYPE_STORE, L1AccessCount, L1MissCount) == false )
    {
        ul2->Access(addr, size, CACHE_BASE::ACCESS_TYPE_STORE, L1AccessCount, L1MissCount);
    }

}

/* ===================================================================== */

VOID LoadSingleFast(ADDRINT addr, UINT64* L1AccessCount, UINT64* L1MissCount, UINT64* L2AccessCount, UINT64* L2MissCount)
{
    
    if( dl1->AccessSingleLine(addr, CACHE_BASE::ACCESS_TYPE_LOAD, L1AccessCount, L1MissCount) == false )
    {
        ul2->AccessSingleLine(addr, CACHE_BASE::ACCESS_TYPE_LOAD, L2AccessCount, L2MissCount);
    }

}

/* ===================================================================== */

VOID StoreSingleFast(ADDRINT addr, UINT64* L1AccessCount, UINT64* L1MissCount, UINT64* L2AccessCount, UINT64* L2MissCount)
{
    
    if( dl1->AccessSingleLine(addr, CACHE_BASE::ACCESS_TYPE_STORE, L1AccessCount, L1MissCount) == false )
    {
        ul2->AccessSingleLine(addr, CACHE_BASE::ACCESS_TYPE_STORE, L2AccessCount, L2MissCount);
    }

}

VOID StoreMultiFast_AxC(ADDRINT addr, UINT32 size, UINT64* L1AccessCount, UINT64* L1MissCount, UINT64* L2AccessCount, UINT64* L2MissCount)
{
    
    if( d_axc->Access(addr, size, CACHE_BASE::ACCESS_TYPE_STORE, L1AccessCount, L1MissCount) == false )
    {
        ul2->Access(addr, size, CACHE_BASE::ACCESS_TYPE_STORE, L1AccessCount, L1MissCount);
    }

}

/* ===================================================================== */

VOID LoadSingleFast_AxC(ADDRINT addr, UINT64* L1AccessCount, UINT64* L1MissCount, UINT64* L2AccessCount, UINT64* L2MissCount)
{
    
    if( d_axc->AccessSingleLine(addr, CACHE_BASE::ACCESS_TYPE_LOAD, L1AccessCount, L1MissCount) == false )
    {
        ul2->AccessSingleLine(addr, CACHE_BASE::ACCESS_TYPE_LOAD, L2AccessCount, L2MissCount);
    }

}

/* ===================================================================== */

VOID StoreSingleFast_AxC(ADDRINT addr, UINT64* L1AccessCount, UINT64* L1MissCount, UINT64* L2AccessCount, UINT64* L2MissCount)
{
    
    if( d_axc->AccessSingleLine(addr, CACHE_BASE::ACCESS_TYPE_STORE, L1AccessCount, L1MissCount) == false )
    {
        ul2->AccessSingleLine(addr, CACHE_BASE::ACCESS_TYPE_STORE, L2AccessCount, L2MissCount);
    }

}



VOID simulate_cacheHierarchy(INS ins, UINT64* functionL1AccessCount, UINT64* functionL1MissCount, UINT64* functionL2AccessCount, UINT64* functionL2MissCount)
{
    
    UINT32 memOperands = INS_MemoryOperandCount(ins);
    
    // Instrument each memory operand. If the operand is both read and written
    // it will be processed twice.
    // Iterating over memory operands ensures that instructions on IA-32 with
    // two read operands (such as SCAS and CMPS) are correctly handled.
    for (UINT32 memOp = 0; memOp < memOperands; memOp++)
    {
        const UINT32 size = INS_MemoryOperandSize(ins, memOp);
        const BOOL   single = (size <= 4);
        
        if (INS_MemoryOperandIsRead(ins, memOp))
        {
            if( single )
            {
                INS_InsertPredicatedCall(
                    ins, IPOINT_BEFORE,  (AFUNPTR) LoadSingleFast_AxC,
                    IARG_MEMORYOP_EA, memOp,
                    IARG_PTR, functionL1AccessCount,
                    IARG_PTR, functionL1MissCount,
                    IARG_PTR, functionL2AccessCount,
                    IARG_PTR, functionL2MissCount,
                    IARG_END);
                    
            }
            else
            {
                INS_InsertPredicatedCall(
                    ins, IPOINT_BEFORE,  (AFUNPTR) LoadMultiFast_AxC,
                    IARG_MEMORYOP_EA, memOp,
                    IARG_UINT32, size,
                    IARG_PTR, functionL1AccessCount,
                    IARG_PTR, functionL1MissCount,
                    IARG_PTR, functionL2AccessCount,
                    IARG_PTR, functionL2MissCount,
                    IARG_END);
            }
        }
        
        if (INS_MemoryOperandIsWritten(ins, memOp))
        {
            if( single )
            {
                INS_InsertPredicatedCall(
                    ins, IPOINT_BEFORE,  (AFUNPTR) StoreSingleFast_AxC,
                    IARG_MEMORYOP_EA, memOp,
                    IARG_PTR, functionL1AccessCount,
                    IARG_PTR, functionL1MissCount,
                    IARG_PTR, functionL2AccessCount,
                    IARG_PTR, functionL2MissCount,
                    IARG_END);
                    
            }
            else
            {
                INS_InsertPredicatedCall(
                    ins, IPOINT_BEFORE,  (AFUNPTR) StoreMultiFast_AxC,
                    IARG_MEMORYOP_EA, memOp,
                    IARG_UINT32, size,
                    IARG_PTR, functionL1AccessCount,
                    IARG_PTR, functionL1MissCount,
                    IARG_PTR, functionL2AccessCount,
                    IARG_PTR, functionL2MissCount,
                    IARG_END);
            }
        }
    }  //for

}
