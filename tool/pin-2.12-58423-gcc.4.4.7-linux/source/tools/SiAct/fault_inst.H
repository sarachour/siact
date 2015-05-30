#include "pin.H"
#include "pin_profile.H"
#include "fault_inject.H"
#include "xed-decoded-inst.h"
#include "info.H"
#include "hierarchy.H"


extern Tuple<PreciseHierarchy,ApproximateHierarchy> * memory;

PreciseHierarchy * precise_mem=0;
ApproximateHierarchy * approx_mem=0;

void* MEMADDR_DEST = 0;
UINT64 MEMADDR_SRC;
UINT32 MEMADDR_SIZE = 0;
BOOL TRANSIENT_ERROR = false;

VOID PIN_FAST_ANALYSIS_CALL siPrecLoadMem(ADDRINT * addrptr, UINT32 size){
	ADDRINT addr = (ADDRINT) addrptr;
	UINT64 data;
	
	precise_mem->load(addr,reinterpret_cast<UINT8*>(&data),size);
}
VOID PIN_FAST_ANALYSIS_CALL siPrecStoreMem(ADDRINT * addrptr, UINT32 size){
	ADDRINT addr = (ADDRINT) addrptr;
	UINT64 data;
	
	precise_mem->store(addr,reinterpret_cast<UINT8*>(&data),size);
		
}

VOID PIN_FAST_ANALYSIS_CALL siApproxLoadMem(ADDRINT * addrptr, UINT32 size){
	ADDRINT addr = (ADDRINT) addrptr;
	BOOL DO_APPROX = approx_mem->approx(addr);
	UINT64 data;
	
	//restore data from last read
	//transient error
	if(TRANSIENT_ERROR){
		PIN_SafeCopy((void*) MEMADDR_DEST, &MEMADDR_SRC,MEMADDR_SIZE); //restore load
	}
	
	
	PIN_SafeCopy(&data,(void *) addr, size);
	
	MEMADDR_DEST = addrptr;
	MEMADDR_SRC = data;
	MEMADDR_SIZE = size;
	
	//printf("load addr=%lx size=%d data=%lx\n", addr, size, data);
	approx_mem->load(addr,reinterpret_cast<UINT8*>(&data), size, DO_APPROX, TRANSIENT_ERROR);
	
	//if(DO_APPROX) 
	PIN_SafeCopy((void *) addr, &data, size);
}
VOID PIN_FAST_ANALYSIS_CALL siApproxStoreMem(ADDRINT * addrptr, UINT32 size){
	ADDRINT addr = (ADDRINT) addrptr;
	UINT64 data;
	BOOL DO_APPROX = approx_mem->approx(addr);
	
	PIN_SafeCopy(&data, (void *) addr, size);
	
	//printf("store addr=%lx size=%d data=%lx\n", addr, size, data);
	approx_mem->store(addr,reinterpret_cast<UINT8*>(&data), size, DO_APPROX);	
	
	//copy back to mem
	PIN_SafeCopy((void *) addr, &data, size);
	//printf("----\n");
}


VOID InstrumentNormalInstruction(INS ins){
		//const UINT32 size = INS_MemoryOperandSize(ins, memOp);
        //const BOOL   single = (size <= 4);
    if(memory == 0) return;
    UINT32 memOperands = INS_MemoryOperandCount(ins);
	//printf("cat: %d ret=%d name=%s ban=%s\n", cat, XED_CATEGORY_RET,CATEGORY_StringShort(cat).c_str(), ban ? "y" : "n");
    if(memory->hasFirst())
		precise_mem = memory->first();
    else {
		approx_mem = memory->second();
	}
        // Iterate over each memory operand of the instruction.
    for (UINT32 memOp = 0; memOp < memOperands; memOp++)
    {
		UINT8 memOpSize = INS_MemoryOperandSize(ins, memOp);
		//found a memory read instance
        if (INS_MemoryOperandIsRead(ins, memOp))
        {
			INS_InsertPredicatedCall(
				ins, IPOINT_BEFORE, (AFUNPTR) (memory->hasFirst() ? siPrecLoadMem : siApproxLoadMem),
				IARG_MEMORYOP_EA, memOp,
				IARG_UINT32, memOpSize,
				IARG_END);
        }
        // Note that in some architectures a single memory operand can be 
        // both read and written (for instance incl (%eax) on IA-32)
        // In that case we instrument it once for read and once for write.
        if (INS_MemoryOperandIsWritten(ins, memOp) && INS_HasFallThrough(ins))
        {
           INS_InsertPredicatedCall(
                ins, IPOINT_AFTER, (AFUNPTR) (memory->hasFirst() ? siPrecStoreMem : siApproxStoreMem),
                IARG_MEMORYOP_EA, memOp,
                IARG_UINT32, memOpSize,
                IARG_END);
        }
    }
    
}

VOID InstrumentTimerInstruction(INS inst){
	INS_InsertCall(inst,
		IPOINT_BEFORE,
		AFUNPTR(timer_inst), IARG_FAST_ANALYSIS_CALL, 
		IARG_END); 	
}


