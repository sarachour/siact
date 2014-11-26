#include "pin.H"
#include "fault_inject.H"
#include "info.H"




#define SINGLE_PREC 1
#define DOUBLE_PREC 2


ADDRINT * MEM_WRITE_PTR;
UINT32 MEM_WRITE_SIZE;
BOOL MEM_ERR_HAPPENED;

VOID PIN_FAST_ANALYSIS_CALL mem_wr_store(ADDRINT * mem, UINT32 size)
{
		MEM_WRITE_PTR = mem;
		MEM_WRITE_SIZE = size;
		
}
INLINE VOID MEM_CORRUPT(task_enum_t TYPE, ADDRINT * mem, UINT32 size, UINT32 PROB){
	if(PROB != 0 && corrupt_mem_softerr(mem, size, PROB)){
		MEM_ERR_HAPPENED = true;
		task_inst(TYPE, true, true);
	}
	task_inst(TYPE, true, false);
}
INLINE VOID MANT_CORRUPT_DBL(task_enum_t TYPE, PIN_REGISTER * val,  UINT32 MASK){
	corrupt_mant_minerva_dbl(val,MASK);
	task_inst(TYPE, true, true);
}
INLINE VOID MANT_CORRUPT_FLT(task_enum_t TYPE, PIN_REGISTER * val,  UINT32 MASK){
	corrupt_mant_minerva_flt(val,MASK);
	task_inst(TYPE, true, true);
}
VOID PIN_FAST_ANALYSIS_CALL mem_wr_corrupt_fp()
{
		MEM_CORRUPT(MEM_WRITE, MEM_WRITE_PTR, MEM_WRITE_SIZE, model.SRAM_WRITE_ERR_FP);
		
}
VOID PIN_FAST_ANALYSIS_CALL mem_rd_corrupt_fp(ADDRINT * mem, UINT32 size)
{
		MEM_CORRUPT(MEM_READ, mem, size, model.SRAM_READ_ERR_FP);
		
}
VOID PIN_FAST_ANALYSIS_CALL mem_wr_corrupt_int()
{
		MEM_CORRUPT(MEM_WRITE, MEM_WRITE_PTR, MEM_WRITE_SIZE, model.SRAM_WRITE_ERR_INT);
		
}
VOID PIN_FAST_ANALYSIS_CALL mem_rd_corrupt_int(ADDRINT * mem, UINT32 size)
{
		MEM_CORRUPT(MEM_READ, mem, size, model.SRAM_READ_ERR_INT);
		
}
VOID PIN_FAST_ANALYSIS_CALL mem_corrupt_dram(ADDRINT * mem, UINT32 size)
{
		bool inj = corrupt_dram(mem,size,model.DRAM_ENABLE);
		task_inst(DRAM, true, inj);
		
}
VOID PIN_FAST_ANALYSIS_CALL mant_corrupt_flt(PIN_REGISTER * val, UINT32 size)
{
		MANT_CORRUPT_FLT(FP, val, model.flt_err_mask);
		
}
VOID PIN_FAST_ANALYSIS_CALL mant_corrupt_dbl(PIN_REGISTER * val, UINT32 size)
{
		MANT_CORRUPT_DBL(FP, val, model.dbl_err_mask);
		
}

VOID PIN_FAST_ANALYSIS_CALL mem_rd_cnt(){
		task_inst(MEM_READ, false, false);
}
VOID PIN_FAST_ANALYSIS_CALL mem_wr_cnt(){
		task_inst(MEM_WRITE, false, false);
}
VOID PIN_FAST_ANALYSIS_CALL mem_cnt(){
		task_inst(MEM, false, false);
}
VOID PIN_FAST_ANALYSIS_CALL mem_instr_cnt(){
		task_inst(MEM, true, MEM_ERR_HAPPENED);
		MEM_ERR_HAPPENED = false;
}
VOID PIN_FAST_ANALYSIS_CALL fp_cnt(){
		task_inst(FP, false, false);
}
VOID PIN_FAST_ANALYSIS_CALL fp_instr_cnt(){
		task_inst(FP, true, true);
}
VOID PIN_FAST_ANALYSIS_CALL rest_cnt(){
		task_inst(TOTAL, false, false);
		timer_inst();
}

BOOL isFPMemInstruction(INS inst){
	UINT32 opcode = INS_Opcode(inst);
	switch(opcode){
		case XED_ICLASS_MOVSS:
		case XED_ICLASS_MOVSD:
		case XED_ICLASS_MOVSD_XMM:
			return true;
		default:
			return false;
	} 
}

BOOL isIntMemInstruction(INS inst){
	UINT32 opcode = INS_Opcode(inst);
	switch(opcode){
		case XED_ICLASS_MOV:
			return true;
		default:
			return false;
	} 
}
bool isFPArithInstruction(INS inst, BOOL * isdbl){
	UINT32 opcode = INS_Opcode(inst);
	switch(opcode){
		case XED_ICLASS_SUBSS:
		case XED_ICLASS_MULSS:
		case XED_ICLASS_ADDSS:
		case XED_ICLASS_SQRTSS:
		case XED_ICLASS_DIVSS:
			*isdbl = false;
			return true;
		case XED_ICLASS_MULSD:
		case XED_ICLASS_ADDSD:
		case XED_ICLASS_SUBSD:
		case XED_ICLASS_SQRTSD:
		case XED_ICLASS_DIVSD:
			*isdbl = true;
			return DOUBLE_PREC;
		default:
			return false;
	} 
}

BOOL isIntArithInstruction(INS inst){
	UINT32 opcode = INS_Opcode(inst);
	switch(opcode){
		case XED_ICLASS_ADD:
		case XED_ICLASS_IMUL:
		case XED_ICLASS_SUB:
		case XED_ICLASS_IDIV:
		case XED_ICLASS_MOV:
			return true;
		default:
			return true;
	} 
}



VOID InstrumentFPInstruction(INS inst){
	//instrument read
	//instrument write	
	bool isdbl;
	bool is_fp_mem = isFPMemInstruction(inst);
	bool is_fp_arith = isFPArithInstruction(inst, &isdbl);
	bool is_int_mem =  isIntMemInstruction(inst);
	bool is_mem_read = INS_IsMemoryRead(inst);
	bool is_mem_read_2 = INS_HasMemoryRead2(inst);
	bool is_mem_write = INS_IsMemoryWrite(inst);
	bool has_fallthrough = INS_HasFallThrough(inst);
	bool is_instr = false;
	bool is_banned = INS_IsProcedureCall(inst) || INS_IsDirectBranchOrCall(inst) ||
		INS_IsIndirectBranchOrCall(inst) || INS_IsSyscall(inst) ||
		INS_IsRet(inst) || INS_IsSysret(inst) || INS_IsPrefetch(inst) ||
		INS_IsCall(inst) || !INS_Valid(inst);

	//dram corrupt
	if(is_mem_read && !is_banned){
		INS_InsertCall(inst,
			 IPOINT_BEFORE,
			 AFUNPTR(mem_corrupt_dram), IARG_FAST_ANALYSIS_CALL, 
			 IARG_MEMORYREAD_EA,
			 IARG_MEMORYREAD_SIZE,
			 IARG_END); 
	}
	if(is_mem_read_2 && !is_banned){
		INS_InsertCall(inst,
			 IPOINT_BEFORE,
			 AFUNPTR(mem_corrupt_dram), IARG_FAST_ANALYSIS_CALL, 
			 IARG_MEMORYREAD2_EA,
			 IARG_MEMORYREAD_SIZE,
			 IARG_END); 
	}
	// Instrument first read
	if(is_mem_read && is_fp_mem){
		INS_InsertCall(inst,
			 IPOINT_BEFORE,
			 AFUNPTR(mem_rd_corrupt_fp), IARG_FAST_ANALYSIS_CALL, 
			 IARG_MEMORYREAD_EA,
			 IARG_MEMORYREAD_SIZE,
			 IARG_END); 
		is_instr = true;
	}
	else if(is_mem_read && is_int_mem && !is_banned){
		INS_InsertCall(inst,
			 IPOINT_BEFORE,
			 AFUNPTR(mem_rd_corrupt_int), IARG_FAST_ANALYSIS_CALL, 
			 IARG_MEMORYREAD_EA,
			 IARG_MEMORYREAD_SIZE,
			 IARG_END); 
		is_instr = true;
	}
	else
		INS_InsertCall(inst,
			 IPOINT_BEFORE,
			 AFUNPTR(mem_rd_cnt), IARG_FAST_ANALYSIS_CALL, 
			 IARG_END); 
								 
	
	// Instrument second read
	if(is_mem_read_2 && is_fp_mem){
		INS_InsertCall(inst,
			 IPOINT_BEFORE,
			 AFUNPTR(mem_rd_corrupt_fp), IARG_FAST_ANALYSIS_CALL, 
			 IARG_MEMORYREAD2_EA,
			 IARG_MEMORYREAD_SIZE,
			 IARG_END); 
		is_instr = true;
	}
	else if(is_mem_read_2 && is_int_mem && !is_banned){
		INS_InsertCall(inst,
			 IPOINT_BEFORE,
			 AFUNPTR(mem_rd_corrupt_int), IARG_FAST_ANALYSIS_CALL, 
			 IARG_MEMORYREAD2_EA,
			 IARG_MEMORYREAD_SIZE,
			 IARG_END); 
		is_instr = true;
	}
	else
		INS_InsertCall(inst,
			 IPOINT_BEFORE,
			 AFUNPTR(mem_rd_cnt), IARG_FAST_ANALYSIS_CALL, 
			 IARG_END); 
	
	if(is_mem_write){
		if(has_fallthrough && (is_fp_mem || is_int_mem)){
			INS_InsertCall(inst,
				 IPOINT_BEFORE,
				 AFUNPTR(mem_wr_store), IARG_FAST_ANALYSIS_CALL, 
				 IARG_MEMORYWRITE_EA,
				 IARG_MEMORYWRITE_SIZE,
				 IARG_END); 
			is_instr = true;
			if(is_fp_mem){
				INS_InsertCall(inst,
							 IPOINT_AFTER,
							 AFUNPTR(mem_wr_corrupt_fp), IARG_FAST_ANALYSIS_CALL,
							 IARG_END); 
			}
			else if(is_int_mem){
				INS_InsertCall(inst,
							 IPOINT_AFTER,
							 AFUNPTR(mem_wr_corrupt_int), IARG_FAST_ANALYSIS_CALL,
							 IARG_END); 
				
			}	
		}
		else { // count if we're not instrumenting it
			INS_InsertCall(inst,
				 IPOINT_BEFORE,
				 AFUNPTR(mem_wr_cnt), IARG_FAST_ANALYSIS_CALL, 
				 IARG_END); 
		}
		
	}		
	if(is_fp_arith && has_fallthrough){
		int n = INS_MaxNumWRegs(inst);
		for(int i=0; i < n; i++){
			REG reg = INS_RegW(inst, i);
			if(isdbl)
				INS_InsertCall(inst,
					 IPOINT_AFTER,
					 AFUNPTR(mant_corrupt_dbl), IARG_FAST_ANALYSIS_CALL,
					 IARG_REG_REFERENCE, (reg),
					 IARG_END); 
			else
				INS_InsertCall(inst,
					 IPOINT_AFTER,
					 AFUNPTR(mant_corrupt_flt), IARG_FAST_ANALYSIS_CALL,
					 IARG_REG_REFERENCE, reg, 
					 IARG_END); 
		}
		is_instr = true;
	}
		 
								 							 		 		 

	if(is_mem_read || is_mem_write || is_mem_read_2){
		if(is_instr)
			INS_InsertCall(inst,
			 IPOINT_BEFORE,
			 AFUNPTR(mem_instr_cnt), IARG_FAST_ANALYSIS_CALL, 
			 IARG_END); 
		else
			INS_InsertCall(inst,
			 IPOINT_BEFORE,
			 AFUNPTR(mem_cnt), IARG_FAST_ANALYSIS_CALL, 
			 IARG_END); 
	}
	else if(is_fp_arith){
		if(is_instr)
			INS_InsertCall(inst,
			 IPOINT_BEFORE,
			 AFUNPTR(fp_instr_cnt), IARG_FAST_ANALYSIS_CALL, 
			 IARG_END); 
		else
			INS_InsertCall(inst,
			 IPOINT_BEFORE,
			 AFUNPTR(fp_cnt), IARG_FAST_ANALYSIS_CALL, 
			 IARG_END); 
	}
	else{
		INS_InsertCall(inst,
			IPOINT_BEFORE,
			AFUNPTR(rest_cnt), IARG_FAST_ANALYSIS_CALL, 
			IARG_END); 
	}

}

VOID InstrumentTimerInstruction(INS inst){
	INS_InsertCall(inst,
		IPOINT_BEFORE,
		AFUNPTR(timer_inst), IARG_FAST_ANALYSIS_CALL, 
		IARG_END); 
	
	
}
