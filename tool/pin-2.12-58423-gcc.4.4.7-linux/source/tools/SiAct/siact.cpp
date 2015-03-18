/*BEGIN_LEGAL 
Intel Open Source License 

Copyright (c) 2002-2013 Intel Corporation. All rights reserved.
 
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.  Redistributions
in binary form must reproduce the above copyright notice, this list of
conditions and the following disclaimer in the documentation and/or
other materials prisInfoStoreFunctionovided with the distribution.  Neither the name of
the Intel Corporation nor the names of its contributors may be used to
endorse or promote products derived from this software without
specific prior written permission.
 
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE INTEL OR
ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
END_LEGAL */

/* ===================================================================== */
/*
  @ORIGINAL_AUTHOR: Robert Muth
  @Modified by: Ramesh Peri
*/

/* ===================================================================== */
/*! @file
 *  This file contains a static and dynamic instruction mix profiler
 */


#include "math.h"
#include "pin.H"
#include "instlib.H"
#include "info.H"
//#include <unistd.h>
#include "portability.H"
#include "hw_fault_model.H"
#include "fault_inst.H"
#include "instr_fxn.H"
#include "hierarchy.H"
#include <vector>
#include <map>
#include <algorithm> // for sort
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string.h>
#include <set>


using namespace INSTLIB;


/* ===================================================================== */



    /* ===================================================================== */

// This function reads the user specified list of AxC functions from a file 



/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */

KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE,         "pintool",
    "o", "insmix.out", "specify file name for insmix profile ");
KNOB<BOOL>   KnobErrorInjection(KNOB_MODE_WRITEONCE, "pintool",
    "e", "0", "Inject Errors");
KNOB<BOOL>   KnobErrorInjectionEverywhere(KNOB_MODE_WRITEONCE, "pintool",
    "w", "0", "Inject Errors everywhere");
/* ===================================================================== */

INT32 Usage()
{
    cerr <<
        "This pin tool injects errors\n"
        "\n";

    cerr << KNOB_BASE::StringKnobSummary();

    cerr << endl;

    return -1;
}

CacheHierarchy * memory;

bool INJECT = false;

/* ===================================================================== */

VOID TASK_START(int id){
	task_start(id);
	//if(task_active_count() > 0) INJECT = true;
}
VOID TASK_STOP(int id, double* dat){
	task_stop(id,dat);
	//if(task_active_count() == 0) INJECT = false;
}

VOID TASK_REFRESH_DRAM(){
	flush_corrupted_dram();
}

VOID task_start_inject(int id){
	INJECT = true;
}
VOID task_stop_inject(int id, double * dat){
	INJECT = false;
}

VOID BindFxns(RTN rtn, VOID *v){
	const char *rtn_name;
	if (!RTN_Valid(rtn)) return;
	
	rtn_name = RTN_Name(rtn).c_str();
	string name = PIN_UndecorateSymbolName(rtn_name, UNDECORATION_NAME_ONLY); // PIN demangles the function names here
	function_type_t ftype = get_function_type(name);
	RTN_Open(rtn);
	switch(ftype){
		case PIN_START_TIMER:
			RTN_InsertCall(rtn, IPOINT_AFTER, (AFUNPTR)timer_start, 
				IARG_FUNCARG_CALLSITE_VALUE, 0, 
				IARG_END);
			break;
		case PIN_STOP_TIMER:
			RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)timer_stop, 
				IARG_FUNCARG_CALLSITE_VALUE, 0, 
				IARG_FUNCARG_CALLSITE_VALUE, 1, 
				IARG_END);
			break;
		case PIN_START_TASK:
			RTN_InsertCall(rtn, IPOINT_AFTER, (AFUNPTR)TASK_START, 
				IARG_FUNCARG_CALLSITE_VALUE, 0, 
				IARG_END);
			break;
		case PIN_STOP_TASK:
			RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)TASK_STOP, 
				IARG_FUNCARG_CALLSITE_VALUE, 0, 
				IARG_FUNCARG_CALLSITE_VALUE, 1, 
				IARG_END);
			break;
		case PIN_DRAM_REFRESH:
			RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)TASK_REFRESH_DRAM, 
				IARG_FUNCARG_CALLSITE_VALUE, 0, 
				IARG_FUNCARG_CALLSITE_VALUE, 1, 
				IARG_END);
			break;
		case PIN_START_INJECT_ERRORS:
			RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)task_start_inject, 
				IARG_FUNCARG_CALLSITE_VALUE, 0, 
				IARG_FUNCARG_CALLSITE_VALUE, 1, 
				IARG_END);
			break;
		case PIN_STOP_INJECT_ERRORS:
			RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)task_stop_inject, 
				IARG_FUNCARG_CALLSITE_VALUE, 0, 
				IARG_FUNCARG_CALLSITE_VALUE, 1, 
				IARG_END);
			break;
		case PIN_BIND_FXN:
			RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)load_hardware_fault_model, 
			IARG_FUNCARG_CALLSITE_VALUE, 0, 
			IARG_END);
			break;
		case PIN_ALLOC_UREL:
			RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)mem_urel_alloc, 
			IARG_FUNCARG_CALLSITE_VALUE, 0,  ///starting address
			IARG_FUNCARG_CALLSITE_VALUE, 1,  //length
			IARG_END);
			break;
		case PIN_LISTED_FUNCTION:
			break;
		default:
			break;
		
	}
	RTN_Close(rtn);
}
VOID Trace(TRACE trace, VOID *v)
{
    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        
        UINT32 numins = 0;
        UINT32 size = 0;

		
		for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins))
		{
			numins += 1;
			size += INS_Size(ins);
			if(INJECT){
				if( INS_IsSyscall(ins) )
				{
					InstrumentTimerInstruction(ins); 
				}
				else
				{
					InstrumentNormalInstruction(ins); 
				}
			}
			
		}
        
        
        // fast-track instruction count for timer is no injection
        INS_InsertCall(BBL_InsHead(bbl), IPOINT_BEFORE, AFUNPTR(timer_inst_m), IARG_FAST_ANALYSIS_CALL, IARG_UINT32, numins, IARG_END);

    }

}



/* ===================================================================== */

VOID Fini(int, VOID * v)
{
    string filename;
    FILE * out;
    // dump insmix profile
    
    filename =  KnobOutputFile.Value();

    filename += "." + decstr( getHWModelIDX())+ ".txt";

    out = fopen(filename.c_str(), "w");
	print_all(out);
	fclose(out);


}


int main(int argc, CHAR *argv[])
{
    PIN_InitSymbols();

    if (PIN_Init(argc, argv))
    {
        return Usage();
    }
    init_all();
    //read hardware fault model file
    readHardwareFaultModelFile();
    //Read the list of AxC functions from axcFunctionList.txt file
    read_function_list();
    
    //insturmentation
    RTN_AddInstrumentFunction(BindFxns, 0);
    TRACE_AddInstrumentFunction(Trace, 0);

    PIN_AddFiniFunction(Fini, 0);

    /*
	DRAM_ALARM.Activate();
	DRAM_ALARM.SetAlarm(INSTRUCTIONS_PER_MILLISECOND, CORRUPT_DRAM_CALLBACK, &DRAM_ALARM);
	*/
	
	// Never returns
	printf("##### STARTING PROGRAM #######\n");
    PIN_StartProgram();
    
    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
