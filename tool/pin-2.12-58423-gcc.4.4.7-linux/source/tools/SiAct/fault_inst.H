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
    UINT32 memOperands = INS_MemoryOperandCount(ins);
    
        // Iterate over each memory operand of the instruction.
    for (UINT32 memOp = 0; memOp < memOperands; memOp++)
    {
		UINT8 memOpSize = INS_MemoryOperandSize(ins, memOp);
        if (INS_MemoryOperandIsRead(ins, memOp))
        {
            INS_InsertPredicatedCall(
                ins, IPOINT_BEFORE, (AFUNPTR)siLoadMem,
                IARG_MEMORYOP_EA, memOp,
                IARG_UINT32, memOpSize,
                IARG_BOOL, INS_IsStackRead(ins) || INS_IsIpRelRead(ins),
                IARG_END);
        }
        // Note that in some architectures a single memory operand can be 
        // both read and written (for instance incl (%eax) on IA-32)
        // In that case we instrument it once for read and once for write.
        if (INS_MemoryOperandIsWritten(ins, memOp) && INS_HasFallThrough(ins))
        {
           INS_InsertPredicatedCall(
                ins, IPOINT_AFTER, (AFUNPTR)siStoreMem,
                IARG_MEMORYOP_EA, memOp,
                IARG_UINT32, memOpSize,
                IARG_BOOL, INS_IsStackWrite(ins) || INS_IsIpRelRead(ins),
                IARG_END);
        }
    }
    
        /*
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
			if(INS_HasFallThrough(ins)){
				INS_InsertPredicatedCall(
					ins, IPOINT_BEFORE,  (AFUNPTR) siRememberStoreMem,
					IARG_MEMORYWRITE_EA,
					IARG_MEMORYWRITE_SIZE,
					IARG_BOOL, INS_IsStackWrite(ins),
					IARG_END);
				INS_InsertPredicatedCall(
					ins, IPOINT_AFTER,  (AFUNPTR) siStoreMem,
					IARG_END);
			}
			else{
				printf("warning: no fallthrough\n");
				INS_InsertPredicatedCall(
					ins, IPOINT_BEFORE,  (AFUNPTR) siRememberStoreMem,
					IARG_MEMORYWRITE_EA,
					IARG_MEMORYWRITE_SIZE,
					IARG_BOOL, INS_IsStackWrite(ins),
					IARG_END);
			}
        }
        */
}

VOID InstrumentTimerInstruction(INS inst){
	INS_InsertCall(inst,
		IPOINT_BEFORE,
		AFUNPTR(timer_inst), IARG_FAST_ANALYSIS_CALL, 
		IARG_END); 	
}


