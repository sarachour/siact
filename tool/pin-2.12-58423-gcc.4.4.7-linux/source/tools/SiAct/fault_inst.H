#include "pin.H"
#include "fault_inject.H"
#include "info.H"
#include "hierarchy.H"


extern Tuple<PreciseHierarchy,ApproximateHierarchy> * memory;

PreciseHierarchy * precise_mem=0;
ApproximateHierarchy * approx_mem=0;


VOID PIN_FAST_ANALYSIS_CALL siPrecLoadMem(ADDRINT * data, UINT32 size, BOOL isStack ){
	printf("prec read %lx size=%d is_stack:%s\n", (UINT64) data, size, isStack ? "y" : "n");
	precise_mem->load(data,size);
}
VOID PIN_FAST_ANALYSIS_CALL siPrecStoreMem(ADDRINT * data, UINT32 size, BOOL isStack){
	printf("prec write %lx size=%d is_stack:%s\n", (UINT64) data, size, isStack ? "y" : "n");
	//approximate store if on stack
	precise_mem->store(data,size);	
}

VOID PIN_FAST_ANALYSIS_CALL siApproxLoadMem(ADDRINT * data, UINT32 size, BOOL isStack ){
	printf("approx read %lx size=%d is_stack:%s\n", (UINT64) data, size, isStack ? "y" : "n");
	approx_mem->load(data,size);
}
VOID PIN_FAST_ANALYSIS_CALL siApproxStoreMem(ADDRINT * data, UINT32 size, BOOL isStack){
	printf("approx write %lx size=%d is_stack:%s\n", (UINT64) data, size, isStack ? "y" : "n");
	//approximate store if on stack
	if(isStack) approx_mem->alloc(data,size); 
	approx_mem->store(data,size);	
}


VOID InstrumentNormalInstruction(INS ins){
		//const UINT32 size = INS_MemoryOperandSize(ins, memOp);
        //const BOOL   single = (size <= 4);
    UINT32 memOperands = INS_MemoryOperandCount(ins);
    BOOL ban = INS_IsBranchOrCall(ins) || INS_IsProcedureCall(ins) || INS_IsRet(ins);
    if(memory->hasFirst())
		precise_mem = memory->first();
    else 
		approx_mem = memory->second();
        // Iterate over each memory operand of the instruction.

    for (UINT32 memOp = 0; memOp < memOperands; memOp++)
    {
		UINT8 memOpSize = INS_MemoryOperandSize(ins, memOp);
        if (INS_MemoryOperandIsRead(ins, memOp))
        {
            INS_InsertPredicatedCall(
                ins, IPOINT_BEFORE, (AFUNPTR) (memory->hasFirst() ? siPrecLoadMem : siApproxLoadMem),
                IARG_MEMORYOP_EA, memOp,
                IARG_UINT32, memOpSize,
                IARG_BOOL, (INS_IsStackRead(ins) || INS_IsIpRelRead(ins)) && !ban,
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
                IARG_BOOL, (INS_IsStackWrite(ins) || INS_IsIpRelRead(ins)) && !ban,
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


