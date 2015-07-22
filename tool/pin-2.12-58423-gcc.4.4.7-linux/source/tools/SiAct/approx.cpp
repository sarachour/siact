#include "approx.H"
#include "stdio.h"
#include "helper.H"
#include "math.h"
#include "xorshift.H"
#include "pin.H"

template <class SET, UINT32 MAX_SETS, UINT32 STORE_ALLOCATION>
APPROXCACHE<SET,MAX_SETS,STORE_ALLOCATION>::APPROXCACHE(string name, UINT32 cacheSize, UINT32 lineSize, UINT32 associativity, ApproximateCacheModel model):
	CACHE_BASE(name, cacheSize, lineSize, associativity)
{
	ASSERTX(NumSets() <= MAX_SETS);

	
	for (UINT32 i = 0; i < NumSets(); i++)
	{
		_sets[i].SetAssociativity(associativity);
	}
	this->model = model;
	ERR_PROB[ACCESS_TYPE_NUM] = 0;
	switch(this->model){
		case CacheModelMedium:
			ERR_PROB[ACCESS_TYPE_LOAD]  = pow(10,-7.4)*RGEN_MAX;
			ERR_PROB[ACCESS_TYPE_STORE]  = pow(10,-4.94)*RGEN_MAX;
			break;
		case CacheModelHeavy:
			ERR_PROB[ACCESS_TYPE_LOAD]  = pow(10,-3)*RGEN_MAX;
			ERR_PROB[ACCESS_TYPE_STORE]  = pow(10,-3)*RGEN_MAX;
			break;
		case CacheModelNone:
			ERR_PROB[ACCESS_TYPE_LOAD]  = 0;
			ERR_PROB[ACCESS_TYPE_STORE] = 0;
			break;
	}
	initApproxStats();
}

template <class SET, UINT32 MAX_SETS, UINT32 STORE_ALLOCATION>
void APPROXCACHE<SET,MAX_SETS,STORE_ALLOCATION>::initApproxStats(){
	acache_stats.TOTAL_APPROX_ACCESSES = 0;
	acache_stats.TOTAL_PRECISE_ACCESSES = 0;
	for(int i=0; i < MAX_SETS; i++){
		acache_stats.TOTAL_APPROX_LINES[i] = 0;
		acache_stats.TOTAL_PRECISE_LINES[i] = 0;
	}
	acache_stats.N = 0;
}

template <class SET, UINT32 MAX_SETS, UINT32 STORE_ALLOCATION>
void APPROXCACHE<SET,MAX_SETS,STORE_ALLOCATION>::updateApproxStats(BOOL isapprox){
	if(isapprox) acache_stats.TOTAL_APPROX_ACCESSES += 1;
	else acache_stats.TOTAL_PRECISE_ACCESSES += 1;
	for(int i=0; i < NumSets(); i++){
		acache_stats.TOTAL_APPROX_LINES[i] += _sets[i].getNApproxLines();
		acache_stats.TOTAL_PRECISE_LINES[i] += _sets[i].getNPreciseLines();
	}
	acache_stats.N++;
}

template <class SET, UINT32 MAX_SETS, UINT32 STORE_ALLOCATION>
void APPROXCACHE<SET,MAX_SETS,STORE_ALLOCATION>::printApproxStats(FILE * out){
	float frac_approx = static_cast<float>(acache_stats.TOTAL_APPROX_ACCESSES)/(acache_stats.TOTAL_APPROX_ACCESSES+acache_stats.TOTAL_PRECISE_ACCESSES);
	fprintf(out,"Approximate Accesses: %ld\n", acache_stats.TOTAL_APPROX_ACCESSES);
	fprintf(out,"Precise Accesses: %ld\n", acache_stats.TOTAL_PRECISE_ACCESSES);
	fprintf(out,"Percent Approximate Accesses: %f%%\n", frac_approx*100.0);
	float TOTAL_APPROX_LINES = 0;
	float TOTAL_PRECISE_LINES = 0;
	for(int i=0; i < NumSets(); i++){
		TOTAL_APPROX_LINES += static_cast<float>(acache_stats.TOTAL_APPROX_LINES[i]);
		TOTAL_PRECISE_LINES += static_cast<float>(acache_stats.TOTAL_PRECISE_LINES[i]);
	}
	frac_approx = TOTAL_APPROX_LINES/(TOTAL_APPROX_LINES + TOTAL_PRECISE_LINES);
	TOTAL_APPROX_LINES /= static_cast<float>(acache_stats.N);
	TOTAL_PRECISE_LINES /= static_cast<float>(acache_stats.N);
	fprintf(out,"Percent Approximate Lines: %f%%\n", frac_approx*100.0);
}

template <class SET, UINT32 MAX_SETS, UINT32 STORE_ALLOCATION>
void APPROXCACHE<SET,MAX_SETS,STORE_ALLOCATION>::Description(FILE * out){
	fprintf(out,"CACHE %s\n", this->Name().c_str());
	fprintf(out,"Line Size: %d\n", this->LineSize());
	fprintf(out,"Cache Size: %d\n", this->CacheSize());
	fprintf(out,"Associativity: %d\n", this->Associativity());
	switch(model){
		case CacheModelMedium: fprintf(out,"Model: %s\n", "Medium"); break;
		case CacheModelHeavy: fprintf(out,"Model: %s\n", "Heavy"); break;
		case CacheModelNone: fprintf(out,"Model: %s\n", "None"); break;
	}
	fprintf(out,"Perr(READ) = %e\n", static_cast<float>(ERR_PROB[ACCESS_TYPE_LOAD])/RGEN_MAX );
	fprintf(out,"Perr(WRITE) = %e\n", static_cast<float>(ERR_PROB[ACCESS_TYPE_STORE])/RGEN_MAX );
}

template <class SET, UINT32 MAX_SETS, UINT32 STORE_ALLOCATION>
bool APPROXCACHE<SET,MAX_SETS,STORE_ALLOCATION>::Access(ADDRINT addr, UINT32 size, ACCESS_TYPE accessType, BOOL SET_APPROX, BOOL& GET_APPROX){
	const ADDRINT highAddr = addr + size;
	const ADDRINT lowAddr = addr;
    bool allHit = true;

    const ADDRINT lineSize = LineSize();
    const ADDRINT notLineMask = ~(lineSize - 1);
    do
    {
        CACHE_TAG tag;
        UINT32 setIndex;

        SplitAddress(addr, tag, setIndex);

        SET & set = _sets[setIndex];

        bool localHit = set.Find(tag, GET_APPROX);
        allHit &= localHit;


        // on miss, loads always allocate, stores optionally
        if ( (! localHit) && (accessType == ACCESS_TYPE_LOAD || STORE_ALLOCATION == STORE_ALLOCATE))
        {
            set.Replace(tag, SET_APPROX);
			updateApproxStats(SET_APPROX);
        }
        else if(localHit && !GET_APPROX && SET_APPROX){
            set.setApprox(tag, SET_APPROX);
			updateApproxStats(SET_APPROX);
		}
		else if(localHit && GET_APPROX && !SET_APPROX){
            set.setApprox(tag, SET_APPROX);
			updateApproxStats(SET_APPROX);
		}
        else{
			updateApproxStats(GET_APPROX);
		}
        addr = (addr & notLineMask) + lineSize; // start of next cache line
    }
    while (addr < highAddr && addr >= lowAddr);

    _access[accessType][allHit]++; //hit/miss count of the cache
    return allHit;
}
/// Cache access at addr that does not span cache lines
template <class SET, UINT32 MAX_SETS, UINT32 STORE_ALLOCATION>
bool APPROXCACHE<SET,MAX_SETS,STORE_ALLOCATION>::AccessSingleLine(ADDRINT addr, ACCESS_TYPE accessType, BOOL SET_APPROX, BOOL& GET_APPROX){
	CACHE_TAG tag;
    UINT32 setIndex;

    SplitAddress(addr, tag, setIndex);

    SET & set = _sets[setIndex];

    bool hit = set.Find(tag, GET_APPROX);


    // on miss, loads always allocate, stores optionally
    if ( (! hit) && (accessType == ACCESS_TYPE_LOAD || STORE_ALLOCATION == STORE_ALLOCATE))
    {
        set.Replace(tag, SET_APPROX);
        updateApproxStats(SET_APPROX);
    }
    else{
		updateApproxStats(GET_APPROX);
	}

    _access[accessType][hit]++; //hit/miss count of the cache

    return hit;
}

template <class SET, UINT32 MAX_SETS, UINT32 STORE_ALLOCATION>
bool APPROXCACHE<SET,MAX_SETS,STORE_ALLOCATION>::ProcessData(UINT8 * data, UINT32 size, ACCESS_TYPE accessType){
	UINT32 PROB = ERR_PROB[accessType];
	if(xorshift32() < PROB){
			UINT64 mask = xorshift64();
			PIN_SafeCopy(data, &mask, size);
			return true;
	}
	return false;
}

template <class SET, UINT32 MAX_SETS, UINT32 STORE_ALLOCATION>
void APPROXCACHE<SET,MAX_SETS,STORE_ALLOCATION>::Report(FILE * out){
	fprintf(out,"%s\n", this->StatsLong("",CACHE_TYPE_DCACHE).c_str());
	printApproxStats(out);
}


#define cell(l,c,r) l*0b100 + c*0b010 + r*0b001

void APPROXMEMORY::initMemoryStats(){
	stats.NREADS = 0;
	stats.NWRITES = 0;
	stats.NCORRUPTIONS = 0;
}
void APPROXMEMORY::updateMemoryStatsReads(bool isread){
		if(isread) stats.NREADS++;
		else stats.NWRITES++;
}
void APPROXMEMORY::updateMemoryStatsCorruption(){
		stats.NCORRUPTIONS++;
}

void APPROXMEMORY::printMemoryStats(FILE * out){
	fprintf(out, "Number Reads: %ld\n", stats.NREADS);
	fprintf(out, "Number Writes: %ld\n", stats.NWRITES);
	fprintf(out, "Number Corruptions: %ld\n", stats.NCORRUPTIONS);
	this->regions->report(out);
}

APPROXMEMORY::APPROXMEMORY(ApproximateMemoryModel model, MEMORY_ADDR_RANGES * regions){
	this->regions = regions;
	this->model = model;
	UINT32 init = 0b00000001;
	for(int i=0; i < 8; i++){
		this->masks[i] = init << i;
	}

	
	this->initMemoryStats();
}	


void APPROXMEMORY::Refresh(){
	this->regions->refresh_all();
}
void APPROXMEMORY::Accumulate(float msec){
	this->regions->accumulate(msec);
}
//return if errors were injected
bool APPROXMEMORY::ProcessData(ADDRINT addr, UINT8 * data, UINT32 size, ACCESS_TYPE accessType){
	uint32_t ncorruptions = 0;
	if(accessType == ACCESS_TYPE_LOAD){
		//if this address is not unreliable, ignore
		updateMemoryStatsReads(true);
		if(!this->regions->contains(addr)) return false;
		
		//get compute msecs since last refresh
		float msec = this->regions->elapsed(addr);
		if(msec == 0) return false;
		if(model == MemoryModelStatic){
			float PROB = 0.0000003*pow(msec,2.6908); // per bit flip probability - double check
			UINT32 IPROB = PROB > 1 ? RAND_MAX : RAND_MAX*PROB; // per bit flip probability
			if(IPROB == 0) return false;
			//printf("P(e)=%e, t=%f, isurel=%s\n",0.0000003*pow(msec,2.6908), msec, this->regions->contains(addr) ? "y" : "n");
			for(UINT32 byte = 0 ; byte < size; byte++){
				for(UINT32 bit = 0; bit < 8; bit++){
					if(xorshift32() < IPROB){
						data[byte]^=masks[bit];
						stats.NCORRUPTIONS++;
						ncorruptions++;
					}
				}
			}
		}
		else if(model == MemoryModelDynamic){ //per byte error probability
			float PROB = 0.0000003*pow(msec,2.6908); // per bit flip probability - double check
			if(PROB == 0) return false;
			for(UINT32 byte = 0 ; byte < size; byte++){
				UINT8 dbyte = data[byte];
				
				for(UINT32 bit = 0; bit < 8; bit++){
					//calculate new probability
					UINT8 val = (dbyte & (1 << bit)) >> bit;
					UINT8 shift = bit-1 < 0 ? 0 : bit-1;
					UINT8 adj = (dbyte&(0b101<<shift))>>shift;
					UINT8 sum = (adj>>2) + (adj&0b001);
					float score = val == 0 ? sum : 2 - sum;
					//7 - 4.5
					float slope = 1.0+score/2.0*1.0;
					float NPROB = PROB*slope;
					float IPROB = NPROB > 1 ? RAND_MAX : RAND_MAX*NPROB;
					ASSERTX(slope >= 1.0);
					if(NPROB > 0 && xorshift32() < IPROB){
						data[byte]^=masks[bit];
						stats.NCORRUPTIONS++;
						ncorruptions++;
					}
				}
			}
		}
		else if(model == MemoryModelTemp){
			float TEMP = -1.5*sin(T) + 2*cos(2*T) + 0.5*cos(1.5*T)*sin(3*T);
			float PROB = TEMP*0.0000003*pow(msec,2.6908); // per bit flip probability - double check
			UINT32 IPROB = PROB > 1 ? RAND_MAX : RAND_MAX*PROB; // per bit flip probability
			
			T+=0.00001;
			if(IPROB == 0) return false;
			
			for(UINT32 byte = 0 ; byte < size; byte++){
				for(UINT32 bit = 0; bit < 8; bit++){
					if(xorshift32() < IPROB){
						data[byte]^=masks[bit];
						stats.NCORRUPTIONS++;
						ncorruptions++;
					}
				}
			}
		}
		else if(model == MemoryModelVariation){
			float TLOC = ((addr>>6)%512)/256.0;
			float PROB = TLOC*0.0000003*pow(msec,2.6908); // per bit flip probability - double check
			UINT32 IPROB = PROB > 1 ? RAND_MAX : RAND_MAX*PROB; // per bit flip probability
			
			for(UINT32 byte = 0 ; byte < size; byte++){
				for(UINT32 bit = 0; bit < 8; bit++){
					if(xorshift32() < IPROB){
						data[byte]^=masks[bit];
						stats.NCORRUPTIONS++;
						ncorruptions++;
					}
				}
			}
		}
	}
	else if(accessType == ACCESS_TYPE_STORE){
		this->regions->refresh(addr);
		updateMemoryStatsReads(false);
	}
	return (ncorruptions > 0);
}

void APPROXMEMORY::Report(FILE * out){
	fprintf(out,"Approximate Memory\n");
	this->printMemoryStats(out);
}
void APPROXMEMORY::Description(FILE * out){
	fprintf(out,"Approximate Memory\n");
	switch(model){
		case MemoryModelDynamic: fprintf(out, "Model: Dynamic\n"); break;
		case MemoryModelStatic: fprintf(out, "Model: Static\n"); break;
		case MemoryModelTemp: fprintf(out, "Model: Temp\n"); break;
		case MemoryModelVariation: fprintf(out, "Model: Variation\n"); break;
		case MemoryModelNone: fprintf(out, "Model: None\n"); break;
	}
}

template class APPROX_CACHE_LRU(16*KILO, 64, STORE_ALLOCATE);
template class APPROX_CACHE_LRU(64 * KILO, 64, STORE_ALLOCATE);
