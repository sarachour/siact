#ifndef STRATEGY_H
#define STRATEGY_H
#include "helper.H"
#include "pin.H"

/*!
 *  @brief Cache set with LRU replacement
 * default is four
 */
template <UINT32 MAX_ASSOCIATIVITY>
class LRU
{
  private:
    vector<CACHE_TAG> _tags;
    vector<UINT64> _tagsPriority;
    UINT32 _tagsLastIndex;
    UINT64 _currentMaxPriority;
  public:
    LRU();
    LRU(UINT32 associativity); //MAX_ASSOCIATIVITY by default
    void SetAssociativity(UINT32 associativity);
    UINT32 GetAssociativity(UINT32 associativity);
    UINT32 Find(CACHE_TAG tag);
    std::string UpdateOwner(CACHE_TAG tag, string newCacheLineOwner);
    void Replace(CACHE_TAG tag);

};

template <UINT32 MAX_ASSOCIATIVITY>
class ALRU
{
  private:
    vector<CACHE_TAG> _tags;
    vector<UINT64> _tagsPriority;
    vector<BOOL> _tagsApprox;
    UINT32 _tagsLastIndex;
    UINT64 _currentMaxPriority;
    UINT32 NAPPROX_LINES;
    UINT32 NPRECISE_LINES;
  public:
    ALRU();
    ALRU(UINT32 associativity); //MAX_ASSOCIATIVITY by default
    void SetAssociativity(UINT32 associativity);
    UINT32 GetAssociativity(UINT32 associativity);
    UINT32 Find(CACHE_TAG tag, BOOL& approx);
    std::string UpdateOwner(CACHE_TAG tag, string newCacheLineOwner);
    void Replace(CACHE_TAG tag, BOOL approx);
    UINT32 getNApproxLines(){return NAPPROX_LINES;}
    UINT32 getNPreciseLines(){return NPRECISE_LINES;}
};




#endif
