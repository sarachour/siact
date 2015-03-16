#ifndef STRATEGY_H
#define STRATEGY_H


/*!
 *  @brief Cache set direct mapped
 */
class DIRECT_MAPPED
{
  private:
    CACHE_TAG _tag;
  public:
    DIRECT_MAPPED(UINT32 associativity = 1);
    VOID SetAssociativity(UINT32 associativity);
    UINT32 GetAssociativity(UINT32 associativity);
    UINT32 Find(CACHE_TAG tag);
    VOID Replace(CACHE_TAG tag);
};

/*!
 *  @brief Cache set with round robin replacement
 */
template <UINT32 MAX_ASSOCIATIVITY = 4>
class ROUND_ROBIN
{
  private:
    CACHE_TAG _tags[MAX_ASSOCIATIVITY];
    UINT32 _tagsLastIndex;
    UINT32 _nextReplaceIndex;

  public:
    ROUND_ROBIN(UINT32 associativity = MAX_ASSOCIATIVITY);
    VOID SetAssociativity(UINT32 associativity);
    UINT32 GetAssociativity(UINT32 associativity);
    UINT32 Find(CACHE_TAG tag);
	string UpdateOwner(CACHE_TAG tag, string newCacheLineOwner);
    VOID Replace(CACHE_TAG tag);
};

/*!
 *  @brief Cache set with LRU replacement
 */
template <UINT32 MAX_ASSOCIATIVITY = 4>
class LRU
{
  private:
    vector<CACHE_TAG> _tags;
    vector<UINT64> _tagsPriority;
    UINT32 _tagsLastIndex;
    UINT64 _currentMaxPriority;

  public:
    LRU(UINT32 associativity = MAX_ASSOCIATIVITY);
    VOID SetAssociativity(UINT32 associativity);
    UINT32 GetAssociativity(UINT32 associativity);
    UINT32 Find(CACHE_TAG tag);
    string UpdateOwner(CACHE_TAG tag, string newCacheLineOwner);
    VOID Replace(CACHE_TAG tag);

};



#endif
