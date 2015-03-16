
/*!
 *  @brief Cache set direct mapped
 */

DIRECT_MAPPED::DIRECT_MAPPED(UINT32 associativity = 1) { 
	ASSERTX(associativity == 1); 
}

VOID DIRECT_MAPPED::SetAssociativity(UINT32 associativity) { 
	ASSERTX(associativity == 1); 
}
UINT32 DIRECT_MAPPED::GetAssociativity(UINT32 associativity) { 
	return 1; 
}

UINT32 DIRECT_MAPPED::Find(CACHE_TAG tag) { 
	return(_tag == tag);
}
VOID DIRECT_MAPPED::Replace(CACHE_TAG tag) { 
	_tag = tag; 
}



ROUND_ROBIN::ROUND_ROBIN(UINT32 associativity = MAX_ASSOCIATIVITY)
  : _tagsLastIndex(associativity - 1)
{
	ASSERTX(associativity <= MAX_ASSOCIATIVITY);
	_nextReplaceIndex = _tagsLastIndex;

	for (INT32 index = _tagsLastIndex; index >= 0; index--)
	{
		_tags[index] = CACHE_TAG(0);
	}
}

VOID ROUND_ROBIN::SetAssociativity(UINT32 associativity)
{
	ASSERTX(associativity <= MAX_ASSOCIATIVITY);
	_tagsLastIndex = associativity - 1;
	_nextReplaceIndex = _tagsLastIndex;
}
UINT32 ROUND_ROBIN::GetAssociativity(UINT32 associativity) { return _tagsLastIndex + 1; }

UINT32 ROUND_ROBIN::Find(CACHE_TAG tag)
{
	bool result = true;

	for (INT32 index = _tagsLastIndex; index >= 0; index--)
	{
		// this is an ugly micro-optimization, but it does cause a
		// tighter assembly loop for ARM that way ...
		if(_tags[index] == tag) goto end;
	}
	result = false;

	end: return result;
}

string ROUND_ROBIN::UpdateOwner(CACHE_TAG tag, string newCacheLineOwner)
{
	for(INT32 index = _tagsLastIndex; index >= 0; index--)
	{
		if(_tags[index] == tag)
		{
			return(_tags[index].updateCacheLineOwner(newCacheLineOwner));
		}
	}
	return("");
}

VOID ROUND_ROBIN::Replace(CACHE_TAG tag)
{
	// g++ -O3 too dumb to do CSE on following lines?!
	const UINT32 index = _nextReplaceIndex;

	_tags[index] = tag;
	// condition typically faster than modulo
	_nextReplaceIndex = (index == 0 ? _tagsLastIndex : index - 1);
}

/*!
 *  @brief Cache set with LRU replacement
*/

LRU::LRU(UINT32 associativity = MAX_ASSOCIATIVITY)
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

VOID LRU::SetAssociativity(UINT32 associativity)
{
  ASSERTX(associativity <= MAX_ASSOCIATIVITY);
  _tagsLastIndex = associativity - 1;
}

UINT32 LRU::GetAssociativity(UINT32 associativity) { return(_tagsLastIndex + 1); }

UINT32 LRU::Find(CACHE_TAG tag)
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

string LRU::UpdateOwner(CACHE_TAG tag, string newCacheLineOwner)
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

VOID LRU::Replace(CACHE_TAG tag)
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

