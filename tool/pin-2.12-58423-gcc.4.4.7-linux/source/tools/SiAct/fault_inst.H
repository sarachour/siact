#include "pin.H"
#include "fault_inject.H"
#include "info.H"
#include "hierarchy.H"



#define SINGLE_PREC 1
#define DOUBLE_PREC 2

extern CacheHierarchy * memory;

VOID PIN_FAST_ANALYSIS_CALL siLoadMem(ADDRINT data, UINT32 size, BOOL isStack ){
	printf("read %lx size=%d is_stack:%s\n", data, size, isStack ? "y" : "n");
}

VOID PIN_FAST_ANALYSIS_CALL siStoreMem(ADDRINT data, UINT32 size, BOOL isStack){
	printf("write %lx size=%d is_stack:%s\n", data, size, isStack ? "y" : "n");
	
}


VOID InstrumentNormalInstruction(INS ins){
		//const UINT32 size = INS_MemoryOperandSize(ins, memOp);
        //const BOOL   single = (size <= 4);
        
        if (INS_IsMemoryRead(ins))
        {
			//multi if size > 4
			INS_InsertPredicatedCall(
				ins, IPOINT_BEFORE,  (AFUNPTR) siLoadMem,
				IARG_MEMORYREAD_EA,
				IARG_MEMORYREAD_SIZE,
				IARG_END);
        }
		
        if(INS_HasMemoryRead2(ins)){
			//multi if size > 4
			INS_InsertPredicatedCall(
				ins, IPOINT_BEFORE,  (AFUNPTR) siLoadMem,
				IARG_MEMORYREAD2_EA,
				IARG_MEMORYREAD_SIZE,
				IARG_BOOL, INS_IsStackRead(ins),
				IARG_END);
		}
		
        if (INS_IsMemoryWrite(ins))
        {
			//modify thing we're writing
            INS_InsertPredicatedCall(
				ins, IPOINT_AFTER,  (AFUNPTR) siStoreMem,
				IARG_MEMORYWRITE_EA,
				IARG_MEMORYWRITE_SIZE,
				IARG_BOOL, INS_IsStackWrite(ins),
				IARG_END);
        }
}

VOID InstrumentTimerInstruction(INS inst){
	INS_InsertCall(inst,
		IPOINT_BEFORE,
		AFUNPTR(timer_inst), IARG_FAST_ANALYSIS_CALL, 
		IARG_END); 	
}


