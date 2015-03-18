#include "pin.H"
#include "fault_inject.H"
#include "info.H"
#include "hierarchy.H"



#define SINGLE_PREC 1
#define DOUBLE_PREC 2

extern CacheHierarchy * memory;

VOID PIN_FAST_ANALYSIS_CALL siLoadMem(ADDRINT * data, UINT32 size){
	
}

VOID PIN_FAST_ANALYSIS_CALL siStoreMem(ADDRINT * data, UINT32 size){
	
}


VOID InstrumentNormalInstruction(INS ins){
		//const UINT32 size = INS_MemoryOperandSize(ins, memOp);
        //const BOOL   single = (size <= 4);
        
        if (INS_IsMemoryRead(ins))
        {
			printf("mem read\n");
			//multi if size > 4
			INS_InsertPredicatedCall(
				ins, IPOINT_BEFORE,  (AFUNPTR) siLoadMem,
				IARG_MEMORYREAD_EA,
				IARG_MEMORYREAD_SIZE,
				IARG_END);
        }
        else if(INS_IsStackRead(ins)){
			printf("stack read\n");
			INS_InsertPredicatedCall(
				ins, IPOINT_BEFORE,  (AFUNPTR) siLoadMem,
				IARG_MEMORYREAD_EA,
				IARG_MEMORYREAD_SIZE,
				IARG_END);
		}
		
        if(INS_HasMemoryRead2(ins)){
			//multi if size > 4
			printf("mem read 2\n");
			INS_InsertPredicatedCall(
				ins, IPOINT_BEFORE,  (AFUNPTR) siLoadMem,
				IARG_MEMORYREAD2_EA,
				IARG_MEMORYREAD_SIZE,
				IARG_END);
		}
		
        if (INS_IsMemoryWrite(ins))
        {
			printf("mem write\n");
            INS_InsertPredicatedCall(
				ins, IPOINT_BEFORE,  (AFUNPTR) siStoreMem,
				IARG_MEMORYWRITE_EA,
				IARG_MEMORYWRITE_SIZE,
				IARG_END);
        }
        else if(INS_IsStackWrite(ins)){
			printf("stack write\n");
			INS_InsertPredicatedCall(
				ins, IPOINT_AFTER,  (AFUNPTR) siStoreMem,
				IARG_MEMORYWRITE_EA,
				IARG_MEMORYWRITE_SIZE,
				IARG_END);
			
		}
}

VOID InstrumentTimerInstruction(INS inst){
	INS_InsertCall(inst,
		IPOINT_BEFORE,
		AFUNPTR(timer_inst), IARG_FAST_ANALYSIS_CALL, 
		IARG_END); 	
}


