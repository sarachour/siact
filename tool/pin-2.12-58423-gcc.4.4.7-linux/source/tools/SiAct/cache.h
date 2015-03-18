
#ifndef PIN_CACHE_H
#define PIN_CACHE_H

#include "helper.H"
#include "strategy.H"
#include "pin.H"
#include <sstream>
#include <algorithm>
#include <vector>
#include <map>
#include <assert.h>



extern "C" {

typedef enum 
{
	STORE_ALLOCATE,
	STORE_NO_ALLOCATE
} STORE_ALLOCATION;

}
/*!
 *  @brief Generic cache base class; no allocate specialization, no cache set specialization
 */
class CACHE_BASE
{
  public:
    // types, constants
    typedef enum 
    {
        ACCESS_TYPE_LOAD,
        ACCESS_TYPE_STORE,
        ACCESS_TYPE_NUM,
        ACCESS_TYPE_APPROX_LOAD,
        ACCESS_TYPE_APPROX_STORE
    } ACCESS_TYPE;
    

    typedef enum
    {
        CACHE_TYPE_ICACHE,
        CACHE_TYPE_DCACHE,
        CACHE_TYPE_NUM
    } CACHE_TYPE;

  protected:
    static const UINT32 HIT_MISS_NUM = 2;
    CACHE_STATS _access[ACCESS_TYPE_NUM][HIT_MISS_NUM];

  private:    // input params
    const std::string _name;
    const UINT32 _cacheSize;
    const UINT32 _lineSize;
    const UINT32 _associativity;

    // computed params
    const UINT32 _lineShift;
    const UINT32 _setIndexMask;

    CACHE_STATS SumAccess(bool hit) const;

  protected:
    UINT32 NumSets() const;

  public:
    // constructors/destructors
    CACHE_BASE(std::string name, UINT32 cacheSize, UINT32 lineSize, UINT32 associativity);

    // accessors
    std::string Name() const {return _name; }
    UINT32 CacheSize() const { return _cacheSize; }
    UINT32 LineSize() const { return _lineSize; }
    UINT32 Associativity() const { return _associativity; }
    //
    CACHE_STATS Hits(ACCESS_TYPE accessType) const { return _access[accessType][true];}
    CACHE_STATS Misses(ACCESS_TYPE accessType) const { return _access[accessType][false];}
    CACHE_STATS Accesses(ACCESS_TYPE accessType) const { return Hits(accessType) + Misses(accessType);}
    CACHE_STATS Hits() const { return SumAccess(true);}
    CACHE_STATS Misses() const { return SumAccess(false);}
    CACHE_STATS Accesses() const { return Hits() + Misses();}
    VOID SplitAddress(const ADDRINT addr, CACHE_TAG & tag, UINT32 & setIndex) const;
    VOID SplitAddress(const ADDRINT addr, CACHE_TAG & tag, UINT32 & setIndex, UINT32 & lineIndex) const;
    string StatsLong(string prefix = "", CACHE_TYPE = CACHE_TYPE_DCACHE) const;
};




/*!
 *  @brief Templated cache class with specific cache set allocation policies
 *
 *  All that remains to be done here is allocate and deallocate the right
 *  type of cache sets.
 */
template <class SET, UINT32 MAX_SETS, UINT32 STORE_ALLOCATION>
class CACHE : public CACHE_BASE
{
  private:
    SET _sets[MAX_SETS];
  public:
	CACHE();
    // constructors/destructors
    CACHE(std::string name, 
		  UINT32 cacheSize, 
		  UINT32 lineSize, 
		  UINT32 associativity);

    // modifiers
    /// Cache access from addr to addr+size-1
    bool Access(ADDRINT addr, UINT32 size, ACCESS_TYPE accessType);
    /// Cache access at addr that does not span cache lines
    bool AccessSingleLine(ADDRINT addr, ACCESS_TYPE accessType);
    void ProcessData(UINT8 * data, ACCESS_TYPE accessType);
    void Report();
    void Description();
};

/*!
 *  @return true if all accessed cache lines hit
 */

// define shortcuts
#define CACHE_LRU(MAX_SETS, MAX_ASSOCIATIVITY, ALLOCATION) CACHE<LRU<MAX_ASSOCIATIVITY>, MAX_SETS, ALLOCATION>

#endif // PIN_CACHE_Ha

