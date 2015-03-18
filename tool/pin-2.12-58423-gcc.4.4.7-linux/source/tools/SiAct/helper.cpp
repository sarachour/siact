#ifndef UTIL_H
#define UTIL_H

#include "helper.H"
#include "string.h"
#include "pin.H"
#include "stdio.h"
#include <iostream>
#include <sstream>
/*! RMR (rodric@gmail.com) 
 *   - temporary work around because decstr()
 *     casts 64 bit ints to 32 bit ones
 */
string local_decstr(UINT64 v, UINT32 w)
{
    ostringstream o;
    o.width(w);
    o << v;
    string str(o.str());
    return str;
}

/*!
 *  @brief Cache tag - self clearing on creation
 */

//CACHE_TAG(ADDRINT tag = 0) { _tag = tag; }
//default: cacheLineOwner = "Default", tag = 0
CACHE_TAG::CACHE_TAG(ADDRINT tag, string cacheLineOwner) { 
	_tag = tag; 
	_cacheLineOwner = cacheLineOwner; 
}

bool CACHE_TAG::operator==(const CACHE_TAG &right) const { 
	return _tag == right._tag; 
}

uint32_t create_mask(int n){
	uint32_t mask = 0;
	for(int i=0; i < n ; i++){
		mask = (0x1<<i) | mask;
	}
	return mask;
}

ADDRINT CACHE_TAG::addr() const { 
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





#endif

