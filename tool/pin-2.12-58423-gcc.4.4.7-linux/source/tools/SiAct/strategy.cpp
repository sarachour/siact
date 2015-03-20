#include "helper.h"
#include "strategy.h"


template <UINT32 MAX_ASSOCIATIVITY>
LRU<MAX_ASSOCIATIVITY>::LRU()
  : _tagsLastIndex(MAX_ASSOCIATIVITY - 1)
{
	UINT32 associativity = MAX_ASSOCIATIVITY;
	ASSERTX(associativity <= MAX_ASSOCIATIVITY);

	_tags.reserve(associativity);
	_tagsPriority.reserve(associativity);
	_currentMaxPriority = 1;
	for (INT32 index = _tagsLastIndex; index >= 0; index--)
	{
		_tags.push_back(CACHE_TAG(0));
		_tagsPriority.push_back(0);
	}
}

template <UINT32 MAX_ASSOCIATIVITY>
LRU<MAX_ASSOCIATIVITY>::LRU(UINT32 associativity)
  : _tagsLastIndex(associativity - 1)
{
	ASSERTX(associativity <= MAX_ASSOCIATIVITY);

	_tags.reserve(associativity);
	_tagsPriority.reserve(associativity);
	_currentMaxPriority = 1;
	for (INT32 index = _tagsLastIndex; index >= 0; index--)
	{
		_tags.push_back(CACHE_TAG(0));
		_tagsPriority.push_back(0);
	}
}

template <UINT32 MAX_ASSOCIATIVITY>
void LRU<MAX_ASSOCIATIVITY>::SetAssociativity(UINT32 associativity)
{
  ASSERTX(associativity <= MAX_ASSOCIATIVITY);
  _tagsLastIndex = associativity - 1;
}

template <UINT32 MAX_ASSOCIATIVITY>
UINT32 LRU<MAX_ASSOCIATIVITY>::GetAssociativity(UINT32 associativity) {
	 return (_tagsLastIndex + 1); 
}

template <UINT32 MAX_ASSOCIATIVITY>
UINT32 LRU<MAX_ASSOCIATIVITY>::Find(CACHE_TAG tag)
{
  bool result = true;

  for (UINT32 index = 0; index <= _tagsLastIndex; ++index)
  {
	if(_tags[index] == tag) 
	{
	
	// _tags.erase(_tags.begin()+index);
	// _tags.insert(_tags.begin(), tempTag);
		_tagsPriority[index] = _currentMaxPriority++;
		goto end;
	}
  }
  result = false;

  end: return result;
}

template <UINT32 MAX_ASSOCIATIVITY>
string LRU<MAX_ASSOCIATIVITY>::UpdateOwner(CACHE_TAG tag, string newCacheLineOwner)
{
  for(UINT32 index = 0; index <= _tagsLastIndex; ++index)
  {
	if(_tags[index] == tag)
	{
		return(_tags[index].updateCacheLineOwner(newCacheLineOwner));
	}
  }
  return("");
}

template <UINT32 MAX_ASSOCIATIVITY>
void LRU<MAX_ASSOCIATIVITY>::Replace(CACHE_TAG tag)
{
  UINT64 minPriority = _tagsPriority[0];
  UINT32 minIndex = 0;
  for (UINT32 index = 1; index <= _tagsLastIndex; ++index)
  {
	if(minPriority > _tagsPriority[index])
	{
		minPriority = _tagsPriority[index];
		minIndex = index;
	}
  }
  _tags[minIndex] = tag;
  _tagsPriority[minIndex] = _currentMaxPriority++;
}


template <UINT32 MAX_ASSOCIATIVITY>
ALRU<MAX_ASSOCIATIVITY>::ALRU()
  : _tagsLastIndex(MAX_ASSOCIATIVITY - 1)
{
	UINT32 associativity = MAX_ASSOCIATIVITY;
	ASSERTX(associativity <= MAX_ASSOCIATIVITY);

	_tags.reserve(associativity);
	_tagsPriority.reserve(associativity);
	_currentMaxPriority = 1;
	NPRECISE_LINES = 0;
	NAPPROX_LINES = 0;
	for (INT32 index = _tagsLastIndex; index >= 0; index--)
	{
		_tags.push_back(CACHE_TAG(0));
		_tagsPriority.push_back(0);
		_tagsApprox.push_back(true);
		NPRECISE_LINES++;
	}
}

template <UINT32 MAX_ASSOCIATIVITY>
ALRU<MAX_ASSOCIATIVITY>::ALRU(UINT32 associativity)
  : _tagsLastIndex(associativity - 1)
{
	ASSERTX(associativity <= MAX_ASSOCIATIVITY);

	_tags.reserve(associativity);
	_tagsPriority.reserve(associativity);
	_currentMaxPriority = 1;
	NPRECISE_LINES = 0;
	NAPPROX_LINES = 0;
	for (INT32 index = _tagsLastIndex; index >= 0; index--)
	{
		_tags.push_back(CACHE_TAG(0));
		_tagsPriority.push_back(0);
		_tagsApprox.push_back(false);
		NPRECISE_LINES++;
	}
}

template <UINT32 MAX_ASSOCIATIVITY>
void ALRU<MAX_ASSOCIATIVITY>::SetAssociativity(UINT32 associativity)
{
  ASSERTX(associativity <= MAX_ASSOCIATIVITY);
  _tagsLastIndex = associativity - 1;
}

template <UINT32 MAX_ASSOCIATIVITY>
UINT32 ALRU<MAX_ASSOCIATIVITY>::GetAssociativity(UINT32 associativity) {
	 return (_tagsLastIndex + 1); 
}

template <UINT32 MAX_ASSOCIATIVITY>
UINT32 ALRU<MAX_ASSOCIATIVITY>::Find(CACHE_TAG tag, BOOL& APPROX)
{
  bool result = true;

  for (UINT32 index = 0; index <= _tagsLastIndex; ++index)
  {
	if(_tags[index] == tag) 
	{
	
	// _tags.erase(_tags.begin()+index);
	// _tags.insert(_tags.begin(), tempTag);
		_tagsPriority[index] = _currentMaxPriority++;
		APPROX = _tagsApprox[index]; //update approximate
		goto end;
	}
  }
  result = false;

  end: return result;
}

template <UINT32 MAX_ASSOCIATIVITY>
string ALRU<MAX_ASSOCIATIVITY>::UpdateOwner(CACHE_TAG tag, string newCacheLineOwner)
{
  for(UINT32 index = 0; index <= _tagsLastIndex; ++index)
  {
	if(_tags[index] == tag)
	{
		return(_tags[index].updateCacheLineOwner(newCacheLineOwner));
	}
  }
  return("");
}

template <UINT32 MAX_ASSOCIATIVITY>
void ALRU<MAX_ASSOCIATIVITY>::Replace(CACHE_TAG tag, BOOL approx)
{
  UINT64 minPriority = _tagsPriority[0];
  UINT32 minIndex = 0;
  for (UINT32 index = 1; index <= _tagsLastIndex; ++index)
  {
	if(minPriority > _tagsPriority[index])
	{
		minPriority = _tagsPriority[index];
		minIndex = index;
	}
  }
  BOOL OLD_APPROX= _tagsApprox[minIndex];
  _tags[minIndex] = tag;
  _tagsPriority[minIndex] = _currentMaxPriority++;
  _tagsApprox[minIndex] = approx;
  
  if(!OLD_APPROX && approx){
	  NAPPROX_LINES += 1;
	  NPRECISE_LINES -= 1;
  }
  if(OLD_APPROX && !approx){
	  NAPPROX_LINES -= 1;
	  NPRECISE_LINES +=1;
  }
}
template <UINT32 MAX_ASSOCIATIVITY>
void ALRU<MAX_ASSOCIATIVITY>::setApprox(CACHE_TAG tag, BOOL approx)
{
  for (UINT32 index = 0; index <= _tagsLastIndex; ++index)
  {
	if(_tags[index] == tag) 
	{
		bool oldApprox = _tagsApprox[index];
		_tagsApprox[index] = approx;
		if(!oldApprox && approx){
			NAPPROX_LINES += 1;
			NPRECISE_LINES -= 1;
		}
		if(oldApprox && !approx){
			NAPPROX_LINES -= 1;
			NPRECISE_LINES +=1;
		}
		return;
	}
  }

  
}

template class LRU<64>;
template class ALRU<64>;
