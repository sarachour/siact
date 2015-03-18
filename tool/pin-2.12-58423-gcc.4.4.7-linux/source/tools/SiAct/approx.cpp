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
	return true;
}
/// Cache access at addr that does not span cache lines
template <class SET, UINT32 MAX_SETS, UINT32 STORE_ALLOCATION>
bool APPROXCACHE<SET,MAX_SETS,STORE_ALLOCATION>::AccessSingleLine(ADDRINT addr, ACCESS_TYPE accessType){
	return true;
}

template <class SET, UINT32 MAX_SETS, UINT32 STORE_ALLOCATION>
void APPROXCACHE<SET,MAX_SETS,STORE_ALLOCATION>::ReadData(UINT8 * data){

}

template <class SET, UINT32 MAX_SETS, UINT32 STORE_ALLOCATION>
void APPROXCACHE<SET,MAX_SETS,STORE_ALLOCATION>::Report(){

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
