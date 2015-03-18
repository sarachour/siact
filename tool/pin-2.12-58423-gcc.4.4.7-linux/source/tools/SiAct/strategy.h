#ifndef STRATEGY_H
#define STRATEGY_H
#include "helper.H"
#include "pin.H"
/*!
 *  @brief Cache set direct mapped
 */
class DIRECT_MAPPED
{
  private:
    CACHE_TAG _tag;
  public:
    DIRECT_MAPPED(UINT32 associativity = 1);
    void SetAssociativity(UINT32 associativity);
    UINT32 GetAssociativity(UINT32 associativity);
    UINT32 Find(CACHE_TAG tag);
    void Replace(CACHE_TAG tag);
};

/*!
 *  @brief Cache set with round robin replacement
 * default is four
 */
template <UINT32 MAX_ASSOCIATIVITY>
class ROUND_ROBIN
{
  private:
    CACHE_TAG _tags[MAX_ASSOCIATIVITY];
    UINT32 _tagsLastIndex;
    UINT32 _nextReplaceIndex;

  public:
    ROUND_ROBIN(UINT32 associativity = MAX_ASSOCIATIVITY);
    void SetAssociativity(UINT32 associativity);
    UINT32 GetAssociativity(UINT32 associativity);
    UINT32 Find(CACHE_TAG tag);
	std::string UpdateOwner(CACHE_TAG tag, string newCacheLineOwner);
    void Replace(CACHE_TAG tag);
};

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



#endif
