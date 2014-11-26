

#define INLINE inline

float TIME_AFTER_LAST_WRITE=0;
float FP_PROB_CORRUPT=0;
uint32_t PROB_CORRUPT=0;
INLINE VOID report_corrupted_dram(){
	TIME_AFTER_LAST_WRITE+=0.001;
	FP_PROB_CORRUPT = 0.0000003*pow(TIME_AFTER_LAST_WRITE,2.6908);
	PROB_CORRUPT = RAND_MAX*(FP_PROB_CORRUPT);
	//printf("time elapsed: time:%f bf-prob:%f\n",TIME_AFTER_LAST_WRITE, FP_PROB_CORRUPT);
}
INLINE VOID flush_corrupted_dram(){
	TIME_AFTER_LAST_WRITE=0;
	FP_PROB_CORRUPT = 0;
	PROB_CORRUPT = 0;
}
INLINE BOOL corrupt_dram(ADDRINT * ptr, UINT32 size, UINT32 DRAM_ENABLE){
	if(TIME_AFTER_LAST_WRITE > 0 && 
		DRAM_ENABLE > 0 &&
		PROB_CORRUPT > 0 && 
		xorshift32() <=PROB_CORRUPT){
			
		UINT32 bit = xorshift64()%(size<<3);
		UINT8 bit_offset = (bit%8);
		UINT8 byte_idx = bit>>3;
		UINT8 data;
		PIN_SafeCopy(&data, &ptr[byte_idx], 1);
		data ^= 0x1<<bit_offset;
		printf("prob: %f, num: %d byte: %d/%d bit: %d/8 %x %lx\n", FP_PROB_CORRUPT, bit, byte_idx,size, bit_offset, data, ptr[byte_idx]);
		PIN_SafeCopy(&ptr[byte_idx], &data, 1);
		return true;
	}
	return false;
	
}
INLINE VOID corrupt_mant_minerva_flt(PIN_REGISTER * ptr, UINT32 MASK)
{
    //corrupt ptr here
    //printf("%e -> ", *((float *) ptr));
    UINT32 data;
    PIN_SafeCopy(&data, ptr->flt, sizeof(FLT32));
    data ^= (data)&MASK;
    PIN_SafeCopy(ptr->flt, &data, sizeof(FLT32));
    //printf("%e\n", *((float *) ptr));
}
INLINE VOID corrupt_mant_minerva_dbl(PIN_REGISTER * ptr,  UINT32 MASK)
{
    //corrupt ptr here
    //printf("%e -> ", *((float *) ptr));
    UINT64 data;
    PIN_SafeCopy(&data, ptr->dbl, sizeof(FLT64));
    data ^= (data)&MASK;
    PIN_SafeCopy(ptr->dbl, &data, sizeof(FLT64));
    //printf("%e\n", *((float *) ptr));
}

INLINE BOOL corrupt_mem_softerr(ADDRINT * ptr, UINT32 size, UINT32 PROB)
{
    //corrupt ptr here
    //corrupt me
    if(xorshift32() < PROB){
			UINT64 mask = xorshift64();
			PIN_SafeCopy(ptr, &mask, size);
			return true;
	}
    return false;
}

