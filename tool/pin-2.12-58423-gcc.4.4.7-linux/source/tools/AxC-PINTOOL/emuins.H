/*BEGIN_LEGAL 
Intel Open Source License 

Copyright (c) 2002-2011 Intel Corporation. All rights reserved.
 
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.  Redistributions
in binary form must reproduce the above copyright notice, this list of
conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution.  Neither the name of
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
/*! @file
 * This tool demonstrates an instruction emulator 
 */

//==============================================================================
//Copyright (c) 2013 Intel Corporation.
//All rights reserved.
//==============================================================================
//
//Developed by:
//
//    Accelerator Architecture Lab, Intel Labs
//
//    Intel Corporation
//
//    http://www.intel.com
//
//Permission is hereby granted, free of charge, to any person obtaining a copy of
//this software and associated documentation files (the "Software"), to deal with
//the Software without restriction, including without limitation the rights to
//use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
//of the Software, and to permit persons to whom the Software is furnished to do
//so, subject to the following conditions:
//
//    * Redistributions of source code must retain the above copyright notice,
//      this list of conditions and the following disclaimers.
//
//    * Redistributions in binary form must reproduce the above copyright notice,
//      this list of conditions and the following disclaimers in the
//      documentation and/or other materials provided with the distribution.
//
//    * Neither the names of the LLVM Team, University of Illinois at
//      Urbana-Champaign, nor the names of its contributors may be used to
//      endorse or promote products derived from this Software without specific
//      prior written permission.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
//FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
//CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS WITH THE
//SOFTWARE.
//==============================================================================
//Contact: Asit K Mishra (asit.k.mishra@intel.com)



/**
    Modified by Sara Achour. 
*/

#include <stdio.h>
#include "pin.H"
#include "pin_isa.H"
//#include "portability.H"
extern "C" {
#include "xed-interface.h"
}
#include <iostream>
#include <iomanip>
#include <fstream>

#include <cstdlib> 
#include "xorshift.h"
#include "saxc.h"
#include "fault_const.H"
//std::ofstream* out = 0;

//#define ProbBitFlips -0.1
//#define ProbBitFlips 1.1


/* ===================================================================== */
/* Handlers                                                              */
/* ===================================================================== */

EXCEPT_HANDLING_RESULT GlobalHandler(THREADID threadIndex, EXCEPTION_INFO * pExceptInfo, 
                                      PHYSICAL_CONTEXT * pPhysCtxt, VOID *v)
{
    // cout << "GlobalHandler: Caught unexpected exception. " << PIN_ExceptionToString(pExceptInfo) << endl;
    return EHR_UNHANDLED;
}


EXCEPT_HANDLING_RESULT DivideHandler(THREADID tid, EXCEPTION_INFO * pExceptInfo, 
                                        PHYSICAL_CONTEXT * pPhysCtxt, VOID *appContextArg)
{
    if(PIN_GetExceptionCode(pExceptInfo) == EXCEPTCODE_INT_DIVIDE_BY_ZERO) 
    {
#if 1
        //Temporary work-around, Remove when Mantis #1986 is resolved
        string str = PIN_ExceptionToString(pExceptInfo);
        printf("GlobalHandler: Caught divide by zero exception. %s\n", str.c_str());
#else
        // cout << "GlobalHandler: Caught divide by zero exception. " << PIN_ExceptionToString(pExceptInfo) << endl;
#endif
        CONTEXT * appCtxt = (CONTEXT *)appContextArg;
        ADDRINT faultIp = PIN_GetContextReg(appCtxt, REG_INST_PTR);
        PIN_SetExceptionAddress(pExceptInfo, faultIp);
        PIN_RaiseException(appCtxt, tid, pExceptInfo); //never returns 
    }
    return EHR_CONTINUE_SEARCH;
}
/*
inline int GenerateRandomNumberInRange(int low, int high)
{
    //int low = -3, high = 3;                                       
    
    int range = (high - low) + 1;                                 
    int random_number;                                            
    random_number = low + int(range * (rand()/(RAND_MAX + 1.0))); 
    return random_number;

}
*/
int N_ERRORS_OCCURRED = 0;
inline int GenerateRandomNumberInRange_0to31()
{                                  
    int random_number;      
    
    //// cout << "Prob specified " << ProbBitFlips << "\n" << endl;
    //Generate a random number between 0 and 1; return a valid number only if generated with the required probability
    
    float val = xorshift32()/(RAND_MAX + 1.0);
    
    if( val <= FAULT_P_BITFLIP_ERROR ){
        random_number = xorshift32()%32;   
        N_ERRORS_OCCURRED++;
	}    
    else
        random_number = -1;
    if(random_number != -1){  
		// cout << "# Errors " << N_ERRORS_OCCURRED << " Flipping: " << random_number<< endl;
	}
    return random_number;

}

inline int GenerateRandomNumberInRange_0to63()
{                                   
    int random_number;                                                  

    //// cout << "Prob specified " << ProbBitFlips << "\n" << endl;
    //Generate a random number between 0 and 1; return a valid number only if generated with the required probability
    
    float val = xorshift32()/(RAND_MAX + 1.0);
    if( val <= FAULT_P_BITFLIP_ERROR ){
        random_number = xorshift32()%64;    
        N_ERRORS_OCCURRED++;
	}   
    else
        random_number = -1;
    
    if(random_number != -1){  
		// cout << "# Errors " << N_ERRORS_OCCURRED<< " Flipping: " << random_number << endl;
	}
    return random_number;

}

//#define XOR_ERROR_SOURCE 
#define FLIPBIT_ERROR_SOURCE
//#define STICK_ERROR_SOURCE

#define MAX_N_BITS_TO_FLIP (8)

//flip random bits
inline void flipRandomBitsFloat(unsigned char * c){
	int i, randomNumber;
	int word,bit;
	for(i=0; i < MAX_N_BITS_TO_FLIP; i++){
		//    //generate a random number between 0 and 31
		randomNumber = GenerateRandomNumberInRange_0to31()%FAULT_N_FLIP_BITS;
		if(randomNumber >= 0){
			word = randomNumber>>3; //div
			bit = randomNumber & 0b111; //mod
			c[word] = c[word] ^ ( 0x1 << bit);
		}
    }
}
inline void flipRandomBitsDouble(unsigned char * c){
	int i, randomNumber;
	int word,bit;
    for(i=0; i < MAX_N_BITS_TO_FLIP; i++){
    //    //generate a random number between 0 and 63 
        randomNumber = GenerateRandomNumberInRange_0to63()%(FAULT_N_FLIP_BITS*2);
        if(randomNumber >= 0){
			word = randomNumber>>3;
			bit = randomNumber & 0b111;
			c[word] = c[word] ^ ( 0x1 << bit);
		}
    }
    
}

inline void flipRandomBitsInt(unsigned char * c){
	flipRandomBitsFloat(c);
}

#define MAX_N_BITS_TO_STICK 8
//flip random bits
inline void stickRandomBitsFloat(unsigned char * c, unsigned char * storeloc){
	int i, randomNumber;
	int word,bit;
	char mask[4];
	mask[0] = mask[1] = mask[2] = mask[3] = 0;
	
	for(i=0; i < MAX_N_BITS_TO_STICK; i++){
		//    //generate a random number between 0 and 31
		randomNumber = GenerateRandomNumberInRange_0to31();
		word = randomNumber>>3;
		bit = randomNumber & 0b111;
		mask[word] = mask[word] | (0x1 << bit);
    }
    c[0] =  (~mask[0]&c[0]) | (mask[0]&storeloc[0]);
    c[1] =  (~mask[1]&c[1]) | (mask[1]&storeloc[1]);
    c[2] =  (~mask[2]&c[2]) | (mask[2]&storeloc[2]);
    c[3] =  (~mask[3]&c[3]) | (mask[3]&storeloc[3]);
}
inline void stickRandomBitsDouble(unsigned char * c, unsigned char * storeloc){
	int i, randomNumber;
	int word,bit;
	char mask[8];
	mask[0] = mask[1] = mask[2] = mask[3] = 0;
	mask[4] = mask[5] = mask[6] = mask[7] =  0;
	
    for(i=0; i < MAX_N_BITS_TO_FLIP; i++){
    //    //generate a random number between 0 and 63 
        randomNumber = GenerateRandomNumberInRange_0to63();
        word = randomNumber>>3;
		bit = randomNumber & 0b111;
		mask[word] = mask[word] | (0x1 << bit);
    }
    c[0] =  (~mask[0]&c[0]) | (mask[0]&storeloc[0]);
    c[1] =  (~mask[1]&c[1]) | (mask[1]&storeloc[1]);
    c[2] =  (~mask[2]&c[2]) | (mask[2]&storeloc[2]);
    c[3] =  (~mask[3]&c[3]) | (mask[3]&storeloc[3]);
    c[4] =  (~mask[4]&c[4]) | (mask[4]&storeloc[4]);
    c[5] =  (~mask[5]&c[5]) | (mask[5]&storeloc[5]);
    c[6] =  (~mask[6]&c[6]) | (mask[6]&storeloc[6]);
    c[7] =  (~mask[7]&c[7]) | (mask[7]&storeloc[7]);
    
}
inline void stickRandomBitsInt(unsigned char * c,unsigned char * s){
	stickRandomBitsFloat(c, s);
}
//xor bits
inline void xor1BitsFloat(unsigned char * c){
	int mask = 0xFF;
    c[0] = c[0] ^ mask; // 8 bits
    c[1] = c[1] ^ mask; //+8 bits
    int mask_1bit = 0x01;
    c[2] = c[2] ^ mask_1bit; //+1 bit
    
}

inline void xor1BitsInt(unsigned char * c){
	int mask = 0xFF;
    c[0] = c[0] ^ mask; // 8 bits
    c[1] = c[1] ^ mask; //+8 bits
    int mask_1bit = 0x01;
    c[2] = c[2] ^ mask_1bit; //+1 bit
    
}

inline void xor1BitsDouble(unsigned char * c){
	//Add noise to all the lower 44 bits of a double value
    int mask = 0xFF;
    c[0] = c[0] ^ mask; // 8 bits
    c[1] = c[1] ^ mask; //+8 bits
    c[2] = c[2] ^ mask; //+8 bits
    c[3] = c[3] ^ mask; //+8 bits
    c[4] = c[4] ^ mask; //+8 bits
    int mask_4bits = 0x0F;
    c[5] = c[5] ^ mask_4bits; //+4 bits
}
/* ===================================================================== */
/* Analysis routine that emulates div                                    */
/* ===================================================================== */
//-------------Divide Instructions--------------------//
VOID EmulateFloatRegDivide(float *reg1Ptr, float *reg2Ptr, CONTEXT * ctxt, THREADID tid)
{
    
    // cout << "Emulating Float Div in Reg" << endl;
    // cout << flush;
    PIN_TryStart(tid, DivideHandler, ctxt);
     
    // cout << " reg1: " << *reg1Ptr << " reg2: " << *reg2Ptr << " Div res: " << (*reg1Ptr/(*reg2Ptr)) << endl;
    float temp = (*reg1Ptr);
    *reg1Ptr = temp/(*reg2Ptr);

    unsigned char *c = reinterpret_cast<unsigned char*>(reg1Ptr);
    
    #ifdef STICK_ERROR_SOURCE
    unsigned char *store = reinterpret_cast<unsigned char*>(&temp);
    stickRandomBitsFloat(c, store);
    #endif
    
    #ifdef FLIPBIT_ERROR_SOURCE
    flipRandomBitsFloat(c);
    #endif
    
    #ifdef XOR_ERROR_SOURCE
    xor1BitsFloat(c);
    #endif
    
    SAxC_IC_COUNTS[SAxC_IC_TYPE_FLOAT_DIV_R_R]++;
    
    PIN_TryEnd(tid);
    // cout << " reg1: " << *reg1Ptr << " reg2: " << *reg2Ptr << endl;

}

VOID EmulateFloatMemDivide(float *reg1Ptr, float *divisorPtr, unsigned int opSize, CONTEXT * ctxt, THREADID tid)
{
    
    // cout << "Emulating Float Div in Mem" << endl;
    // cout << flush;
    float divisor = 0;
    PIN_SafeCopy(&divisor, divisorPtr, opSize);

    PIN_TryStart(tid, DivideHandler, ctxt);
     
    // cout << " reg1: " << *reg1Ptr << " mem: " << *divisorPtr /*<< " Div res: " << (*reg1Ptr/(*reg2Ptr)) */<< endl;
    float temp = (*reg1Ptr);
    *reg1Ptr = temp/divisor;

    unsigned char *c = reinterpret_cast<unsigned char*>(reg1Ptr);
    
    #ifdef STICK_ERROR_SOURCE
    unsigned char *store = reinterpret_cast<unsigned char*>(&temp);
    stickRandomBitsFloat(c, store);
    #endif
    
    #ifdef FLIPBIT_ERROR_SOURCE
    flipRandomBitsFloat(c);
    #endif
    
    #ifdef XOR_ERROR_SOURCE
    xor1BitsFloat(c);
    #endif
    
    SAxC_IC_COUNTS[SAxC_IC_TYPE_FLOAT_DIV_R_M]++;
    
    PIN_TryEnd(tid);
    // cout << " reg1: " << *reg1Ptr << " mem: " << *divisorPtr << endl;

}

VOID EmulateIntRegDivide(int *reg1Ptr, int *reg2Ptr, int * result, CONTEXT * ctxt, THREADID tid)
{
    
    // cout << "Emulating Float Div in Reg" << endl;
    // cout << flush;
    PIN_TryStart(tid, DivideHandler, ctxt);
     
    //out << " reg1: " << *reg1Ptr << " reg2: " << *reg2Ptr << " Div res: " << (*reg1Ptr/(*reg2Ptr)) << endl;
    int temp = *reg1Ptr/(*reg2Ptr);

    unsigned char *c = reinterpret_cast<unsigned char*>(&temp);
    
    #ifdef STICK_ERROR_SOURCE
    unsigned char *store = reinterpret_cast<unsigned char*>(result);
    stickRandomBitsInt(c, store);
    #endif
    
    #ifdef FLIPBIT_ERROR_SOURCE
    flipRandomBitsInt(c);
    #endif
    
    #ifdef XOR_ERROR_SOURCE
    xor1BitsInt(c);
    #endif
    
    *result = temp;
    
    SAxC_IC_COUNTS[SAxC_IC_TYPE_INT_DIV_R_R]++;
    
    PIN_TryEnd(tid);
    //cout << " reg1: " << *reg1Ptr << " reg2: " << *reg2Ptr << endl;

}

VOID EmulateIntMemDivide(int * dividend, int * divisorAddr, int * result, unsigned int opSize, CONTEXT * ctxt, THREADID tid)
{
    int divisor = 0;
    PIN_SafeCopy(&divisor, divisorAddr, opSize);
	
	PIN_TryStart(tid, DivideHandler, ctxt);
     
	//int temp = *result;
	int temp = *dividend/divisor;
	
    unsigned char *c = reinterpret_cast<unsigned char*>(&temp);
    
    #ifdef STICK_ERROR_SOURCE
    unsigned char *store = reinterpret_cast<unsigned char*>(result);
    stickRandomBitsInt(c, store);
    #endif
    
    #ifdef FLIPBIT_ERROR_SOURCE
    flipRandomBitsInt(c);
    #endif
    
    #ifdef XOR_ERROR_SOURCE
    xor1BitsInt(c);
    #endif
    *result = temp;
    SAxC_IC_COUNTS[SAxC_IC_TYPE_INT_DIV_R_M]++;
    
    PIN_TryEnd(tid);

}

VOID EmulateDoubleRegDivide(double *reg1Ptr, double *reg2Ptr, CONTEXT * ctxt, THREADID tid)
{
    
    // cout << "Emulating Double Div in Regs" << endl;
    // cout << flush;
    PIN_TryStart(tid, DivideHandler, ctxt);
     
    // cout << " reg1: " << *reg1Ptr << " reg2: " << *reg2Ptr << " Div res: " << (*reg1Ptr/(*reg2Ptr)) << endl;
    double temp = (*reg1Ptr);
    *reg1Ptr = temp/(*reg2Ptr);

    unsigned char *c = reinterpret_cast<unsigned char*>(reg1Ptr);

    //Add noise to all the lower 44 bits of a double value    
    #ifdef STICK_ERROR_SOURCE
    unsigned char *store = reinterpret_cast<unsigned char*>(&temp);
    stickRandomBitsDouble(c, store);
    #endif
    
    #ifdef FLIPBIT_ERROR_SOURCE
    flipRandomBitsDouble(c);
    #endif
    
    #ifdef XOR_ERROR_SOURCE
    xor1BitsDouble(c);
    #endif
	
	SAxC_IC_COUNTS[SAxC_IC_TYPE_DOUBLE_DIV_R_R]++;
    
    PIN_TryEnd(tid);
    // cout << " reg1: " << *reg1Ptr << " reg2: " << *reg2Ptr << endl;

}

VOID EmulateDoubleMemDivide(double *reg1Ptr, double *divisorPtr, unsigned int opSize, CONTEXT * ctxt, THREADID tid)
{
    
    // cout << "Emulating Double Div in Mem" << endl;
    // cout << flush;
    double divisor;
    PIN_SafeCopy(&divisor, divisorPtr, opSize);

    PIN_TryStart(tid, DivideHandler, ctxt);
     
    // cout << " reg1: " << *reg1Ptr << " mem: " << *divisorPtr /*<< " Div res: " << (*reg1Ptr/(*reg2Ptr)) */<< endl;
    double temp = (*reg1Ptr);
    *reg1Ptr = temp/divisor;

    unsigned char *c = reinterpret_cast<unsigned char*>(reg1Ptr);

    //Add noise to all the lower 44 bits of a double value    
    #ifdef STICK_ERROR_SOURCE
    unsigned char *store = reinterpret_cast<unsigned char*>(&temp);
    stickRandomBitsDouble(c, store);
    #endif
    
    #ifdef FLIPBIT_ERROR_SOURCE
    flipRandomBitsDouble(c);
    #endif
    
    #ifdef XOR_ERROR_SOURCE
    xor1BitsDouble(c);
    #endif
    
    SAxC_IC_COUNTS[SAxC_IC_TYPE_DOUBLE_DIV_R_M]++;
    
    PIN_TryEnd(tid);
    // cout << " reg1: " << *reg1Ptr << " mem: " << *divisorPtr << endl;

}

/* ===================================================================== */
/* Analysis routine that emulates mul                                    */
/* ===================================================================== */
//-------------Multiply Instructions--------------------//
VOID EmulateFloatRegMultiply(float *reg1Ptr, float *reg2Ptr, CONTEXT * ctxt, THREADID tid)
{
    
    // cout << "Emulating Float Mul in Regs" << endl;
    // cout << flush;
     
    // cout << " reg1: " << *reg1Ptr << " reg2: " << *reg2Ptr << " Mul res: " << (*reg1Ptr * (*reg2Ptr)) << endl;
    float temp = (*reg1Ptr); 
    *reg1Ptr = temp * (*reg2Ptr);

    unsigned char *c = reinterpret_cast<unsigned char*>(reg1Ptr);
    
    #ifdef STICK_ERROR_SOURCE
    unsigned char *store = reinterpret_cast<unsigned char*>(&temp);
    stickRandomBitsFloat(c, store);
    #endif
    
    #ifdef FLIPBIT_ERROR_SOURCE
    flipRandomBitsFloat(c);
    #endif
    
    #ifdef XOR_ERROR_SOURCE
    xor1BitsFloat(c);
    #endif
    
    SAxC_IC_COUNTS[SAxC_IC_TYPE_FLOAT_MUL_R_R]++;
    // cout << " reg1: " << *reg1Ptr << " reg2: " << *reg2Ptr << endl;

}

VOID EmulateFloatMemMultiply(float *reg1Ptr, float *divisorPtr, unsigned int opSize, CONTEXT * ctxt, THREADID tid)
{
    
    // cout << "Emulating Float Mul in Mem" << endl;
    // cout << flush;
    float divisor = 0;
    PIN_SafeCopy(&divisor, divisorPtr, opSize);
     
    // cout << " reg1: " << *reg1Ptr << " mem: " << *divisorPtr /*<< " Mul res: " << (*reg1Ptr * (*reg2Ptr)) */<< endl;
    float temp = (*reg1Ptr);
    *reg1Ptr = temp * divisor;

    unsigned char *c = reinterpret_cast<unsigned char*>(reg1Ptr);
    
    #ifdef STICK_ERROR_SOURCE
    unsigned char *store = reinterpret_cast<unsigned char*>(&temp);
    stickRandomBitsFloat(c, store);
    #endif
    
    #ifdef FLIPBIT_ERROR_SOURCE
    flipRandomBitsFloat(c);
    #endif
    
    #ifdef XOR_ERROR_SOURCE
    xor1BitsFloat(c);
    #endif
	SAxC_IC_COUNTS[SAxC_IC_TYPE_FLOAT_MUL_R_M]++;
   
    // cout << " reg1: " << *reg1Ptr << " mem: " << *divisorPtr << endl;

}

VOID EmulateDoubleRegMultiply(double *reg1Ptr, double *reg2Ptr, CONTEXT * ctxt, THREADID tid)
{
    
    // cout << "Emulating Double Mul in Regs" << endl;
    // cout << flush;
    // cout << " reg1: " << *reg1Ptr << " reg2: " << *reg2Ptr << " Mul res: " << (*reg1Ptr * (*reg2Ptr)) << endl;
    double temp = (*reg1Ptr); 
    *reg1Ptr = temp * (*reg2Ptr);

    unsigned char *c = reinterpret_cast<unsigned char*>(reg1Ptr);
    
    #ifdef STICK_ERROR_SOURCE
    unsigned char *store = reinterpret_cast<unsigned char*>(&temp);
    stickRandomBitsDouble(c, store);
    #endif
    
    //Add noise to all the lower 44 bits of a double value
    #ifdef FLIPBIT_ERROR_SOURCE
    flipRandomBitsDouble(c);
    #endif
    
    #ifdef XOR_ERROR_SOURCE
    xor1BitsDouble(c);
    #endif
    SAxC_IC_COUNTS[SAxC_IC_TYPE_DOUBLE_MUL_R_R]++;
    // cout << " reg1: " << *reg1Ptr << " reg2: " << *reg2Ptr << endl;

}

VOID EmulateDoubleMemMultiply(double *reg1Ptr, double *divisorPtr, unsigned int opSize, CONTEXT * ctxt, THREADID tid)
{
    
    // cout << "Emulating Double Mul in Mem" << endl;
    // cout << flush;
    double divisor;
    PIN_SafeCopy(&divisor, divisorPtr, opSize);

    // cout << " reg1: " << *reg1Ptr << " mem: " << *divisorPtr /*<< " Mul res: " << (*reg1Ptr * (*reg2Ptr)) */<< endl;
    double temp = (*reg1Ptr); 
    *reg1Ptr = temp * divisor;

    unsigned char *c = reinterpret_cast<unsigned char*>(reg1Ptr);
    
    #ifdef STICK_ERROR_SOURCE
    unsigned char *store = reinterpret_cast<unsigned char*>(&temp);
    stickRandomBitsDouble(c, store);
    #endif
    //Add noise to all the lower 44 bits of a double value
    #ifdef FLIPBIT_ERROR_SOURCE
    flipRandomBitsDouble(c);
    #endif
    
    #ifdef XOR_ERROR_SOURCE
    xor1BitsDouble(c);
    #endif
	SAxC_IC_COUNTS[SAxC_IC_TYPE_DOUBLE_MUL_R_M]++;
    // cout << " reg1: " << *reg1Ptr << " mem: " << *divisorPtr << endl;

}

VOID EmulateIntRegMultiply(int *reg1Ptr, int *reg2Ptr, CONTEXT * ctxt, THREADID tid)
{
    
    //cout << "Emulating Int Mul in Regs" << endl;
    // cout << flush;
     
    //cout << " reg1: " << *reg1Ptr << " reg2: " << *reg2Ptr << " Mul res: " << (*reg1Ptr * (*reg2Ptr)) << endl;
    int temp = (*reg1Ptr); 
    *reg1Ptr = temp * (*reg2Ptr);

    unsigned char *c = reinterpret_cast<unsigned char*>(reg1Ptr);
    
    #ifdef STICK_ERROR_SOURCE
    unsigned char *store = reinterpret_cast<unsigned char*>(&temp);
    stickRandomBitsInt(c, store);
    #endif
    
    #ifdef FLIPBIT_ERROR_SOURCE
    flipRandomBitsInt(c);
    #endif
    
    #ifdef XOR_ERROR_SOURCE
    xor1BitsInt(c);
    #endif
    
    SAxC_IC_COUNTS[SAxC_IC_TYPE_INT_MUL_R_R]++;
    // cout << " reg1: " << *reg1Ptr << " reg2: " << *reg2Ptr << endl;

}

VOID EmulateIntMemMultiply(int *reg1Ptr, int *divisorPtr, unsigned int opSize, CONTEXT * ctxt, THREADID tid)
{
    
    //cout << "Emulating Int Mul in Mem" << endl;
    // cout << flush;
    int divisor = 0;
    PIN_SafeCopy(&divisor, divisorPtr, opSize);
     
    // cout << " reg1: " << *reg1Ptr << " mem: " << *divisorPtr /*<< " Mul res: " << (*reg1Ptr * (*reg2Ptr)) */<< endl;
    float temp = (*reg1Ptr);
    *reg1Ptr = temp * divisor;

    unsigned char *c = reinterpret_cast<unsigned char*>(reg1Ptr);
    
    #ifdef STICK_ERROR_SOURCE
    unsigned char *store = reinterpret_cast<unsigned char*>(&temp);
    stickRandomBitsInt(c, store);
    #endif
    
    #ifdef FLIPBIT_ERROR_SOURCE
    flipRandomBitsInt(c);
    #endif
    
    #ifdef XOR_ERROR_SOURCE
    xor1BitsInt(c);
    #endif
	SAxC_IC_COUNTS[SAxC_IC_TYPE_INT_MUL_R_M]++;
   
    // cout << " reg1: " << *reg1Ptr << " mem: " << *divisorPtr << endl;

}

/* ===================================================================== */
/* Analysis routine that emulates add                                    */
/* ===================================================================== */
//-------------Add Instructions--------------------//
VOID EmulateFloatRegSum(float *reg1Ptr, float *reg2Ptr, CONTEXT * ctxt, THREADID tid)
{
    
    // cout << "Emulating Float Sum in Regs" << endl;
    // cout << flush;
     
    // cout << " reg1: " << *reg1Ptr << " reg2: " << *reg2Ptr << " Sum res: " << (*reg1Ptr + (*reg2Ptr)) << endl;
    float temp = (*reg1Ptr); 
    *reg1Ptr = temp + (*reg2Ptr);

    unsigned char *c = reinterpret_cast<unsigned char*>(reg1Ptr);

    
    #ifdef STICK_ERROR_SOURCE
    unsigned char *store = reinterpret_cast<unsigned char*>(&temp);
    stickRandomBitsFloat(c, store);
    #endif
    //Add 8 bits of noise anywhere
    #ifdef FLIPBIT_ERROR_SOURCE
    flipRandomBitsFloat(c);
    #endif
    
    #ifdef XOR_ERROR_SOURCE
    xor1BitsFloat(c);
    #endif
	SAxC_IC_COUNTS[SAxC_IC_TYPE_FLOAT_ADD_R_R]++;
   
    // cout << " reg1: " << *reg1Ptr << " reg2: " << *reg2Ptr << " Sum res: " << (*reg1Ptr + (*reg2Ptr)) << endl;

}

VOID EmulateFloatMemSum(float *reg1Ptr, float *divisorPtr, unsigned int opSize, CONTEXT * ctxt, THREADID tid)
{
    
    // cout << "Emulating Float Sum in Mem" << endl;
    // cout << flush;
    float divisor = 0;
    PIN_SafeCopy(&divisor, divisorPtr, opSize);

    // cout << " reg1: " << *reg1Ptr << " mem: " << *divisorPtr /*<< " Sum res: " << (*reg1Ptr + (*reg2Ptr)) */<< endl;
    float temp = (*reg1Ptr); 
    *reg1Ptr = temp + divisor;

    unsigned char *c = reinterpret_cast<unsigned char*>(reg1Ptr);
    
    #ifdef STICK_ERROR_SOURCE
    unsigned char *store = reinterpret_cast<unsigned char*>(&temp);
    stickRandomBitsFloat(c, store);
    #endif
    //Add noise to all the lower 17 bits of a float value
    //Add 8 bits of noise anywhere
    #ifdef FLIPBIT_ERROR_SOURCE
    flipRandomBitsFloat(c);
    #endif
    
    #ifdef XOR_ERROR_SOURCE
    xor1BitsFloat(c);
    #endif
	SAxC_IC_COUNTS[SAxC_IC_TYPE_FLOAT_ADD_R_M]++;
    // cout << " reg1: " << *reg1Ptr << " mem: " << *divisorPtr /*<< " Sum res: " << (*reg1Ptr + (*reg2Ptr)) */<< endl;

}

VOID EmulateDoubleRegSum(double *reg1Ptr, double *reg2Ptr, CONTEXT * ctxt, THREADID tid)
{
    
    // cout << "Emulating Double Sum in Regs" << endl;
    // cout << flush;
     
    // cout << " reg1: " << *reg1Ptr << " reg2: " << *reg2Ptr << " Mul res: " << (*reg1Ptr + (*reg2Ptr)) << endl;
    double temp = (*reg1Ptr); //*2;
    *reg1Ptr = temp + (*reg2Ptr);

    unsigned char *c = reinterpret_cast<unsigned char*>(reg1Ptr);
    
    #ifdef STICK_ERROR_SOURCE
    unsigned char *store = reinterpret_cast<unsigned char*>(&temp);
    stickRandomBitsDouble(c, store);
    #endif
    //Add noise to all the lower 44 bits of a double value
    #ifdef FLIPBIT_ERROR_SOURCE
    flipRandomBitsDouble(c);
    #endif
    
    #ifdef XOR_ERROR_SOURCE
    xor1BitsDouble(c);
    #endif
	SAxC_IC_COUNTS[SAxC_IC_TYPE_DOUBLE_ADD_R_R]++;
    
    // cout << " reg1: " << *reg1Ptr << " reg2: " << *reg2Ptr << endl;

}

VOID EmulateDoubleMemSum(double *reg1Ptr, double *divisorPtr, unsigned int opSize, CONTEXT * ctxt, THREADID tid)
{
    
    // cout << "Emulating Double Mul in Mem" << endl;
    // cout << flush;
    double divisor;
    PIN_SafeCopy(&divisor, divisorPtr, opSize);

    // cout << " reg1: " << *reg1Ptr << " mem: " << *divisorPtr /*<< " Sum res: " << (*reg1Ptr + (*reg2Ptr)) */<< endl;
    double temp = (*reg1Ptr); 
    *reg1Ptr = temp + divisor;

    unsigned char *c = reinterpret_cast<unsigned char*>(reg1Ptr);
    
    #ifdef STICK_ERROR_SOURCE
    unsigned char *store = reinterpret_cast<unsigned char*>(&temp);
    stickRandomBitsDouble(c, store);
    #endif
    //Add noise to all the lower 44 bits of a double value
    #ifdef FLIPBIT_ERROR_SOURCE
    flipRandomBitsDouble(c);
    #endif
    
    #ifdef XOR_ERROR_SOURCE
    xor1BitsDouble(c);
    #endif
	SAxC_IC_COUNTS[SAxC_IC_TYPE_DOUBLE_ADD_R_M]++;
    
    // cout << " reg1: " << *reg1Ptr << " mem: " << *divisorPtr /*<< " Sum res: " << (*reg1Ptr + (*reg2Ptr)) */<< endl;

}

//-------------Add Instructions--------------------//
VOID EmulateIntSum_R_R(int *reg1Ptr, int *reg2Ptr, CONTEXT * ctxt, THREADID tid)
{
    
    //cout << "Emulating Int Sum in Regs" << endl;
    // cout << flush;
     
    // cout << " reg1: " << *reg1Ptr << " reg2: " << *reg2Ptr << " Sum res: " << (*reg1Ptr + (*reg2Ptr)) << endl;
    int temp = (*reg1Ptr); 
    *reg1Ptr = temp + (*reg2Ptr);

    unsigned char *c = reinterpret_cast<unsigned char*>(reg1Ptr);

    
    #ifdef STICK_ERROR_SOURCE
    unsigned char *store = reinterpret_cast<unsigned char*>(&temp);
    stickRandomBitsInt(c, store);
    #endif
    //Add 8 bits of noise anywhere
    #ifdef FLIPBIT_ERROR_SOURCE
    flipRandomBitsInt(c);
    #endif
    
    #ifdef XOR_ERROR_SOURCE
    xor1BitsInt(c);
    #endif
	SAxC_IC_COUNTS[SAxC_IC_TYPE_INT_ADD_R_R]++;
   
    // cout << " reg1: " << *reg1Ptr << " reg2: " << *reg2Ptr << " Sum res: " << (*reg1Ptr + (*reg2Ptr)) << endl;

}

VOID EmulateIntSum_R_M(int *reg1Ptr, int *divisorPtr, unsigned int opSize, CONTEXT * ctxt, THREADID tid)
{
    
    //cout << "Emulating Int Sum in Mem" << endl;
    // cout << flush;
    int divisor = 0;
    PIN_SafeCopy(&divisor, divisorPtr, opSize);

    //cout << " reg1: " << *reg1Ptr << " mem: " << divisor << " Sum res: " << (*reg1Ptr + (divisor)) << endl;
    int temp = (*reg1Ptr); 
    *reg1Ptr = temp + divisor;

    unsigned char *c = reinterpret_cast<unsigned char*>(reg1Ptr);
    
    #ifdef STICK_ERROR_SOURCE
    unsigned char *store = reinterpret_cast<unsigned char*>(&temp);
    stickRandomBitsFloat(c, store);
    #endif
    //Add noise to all the lower 17 bits of a float value
    //Add 8 bits of noise anywhere
    #ifdef FLIPBIT_ERROR_SOURCE
    flipRandomBitsFloat(c);
    #endif
    
    #ifdef XOR_ERROR_SOURCE
    xor1BitsFloat(c);
    #endif
	SAxC_IC_COUNTS[SAxC_IC_TYPE_INT_ADD_R_M]++;
    //cout << " reg1: " << *reg1Ptr << " mem: " << *divisorPtr /*<< " Sum res: " << (*reg1Ptr + (*reg2Ptr)) */<< endl<<endl;

}


VOID EmulateIntSum_M_R(int *reg1Ptr, int *memPtr, unsigned int opSize, CONTEXT * ctxt, THREADID tid)
{
    
    //cout << "Emulating Int Sum in Mem" << endl;
    // cout << flush;
    int mem = 0;
    PIN_SafeCopy(&mem, memPtr, opSize);

    //cout << " reg1: " << *reg1Ptr << " mem: " << mem << " Sum res: " << (*reg1Ptr + (mem)) << endl;
    int temp = *reg1Ptr + mem;

    unsigned char *c = reinterpret_cast<unsigned char*>(&temp);
    
    #ifdef STICK_ERROR_SOURCE
    unsigned char *store = reinterpret_cast<unsigned char*>(&mem);
    stickRandomBitsFloat(c, store);
    #endif
    //Add noise to all the lower 17 bits of a float value
    //Add 8 bits of noise anywhere
    #ifdef FLIPBIT_ERROR_SOURCE
    flipRandomBitsFloat(c);
    #endif
    
    #ifdef XOR_ERROR_SOURCE
    xor1BitsFloat(c);
    #endif
    
    PIN_SafeCopy(memPtr, &temp, opSize);
    
    
	SAxC_IC_COUNTS[SAxC_IC_TYPE_INT_ADD_R_M]++;
    //cout << " reg1: " << *reg1Ptr << " mem: " << *memPtr /*<< " Sum res: " << (*reg1Ptr + (*reg2Ptr)) */<< endl<<endl;

}


/* ===================================================================== */
/* Analysis routine that emulates sub                                    */
/* ===================================================================== */
//-------------Sub Instructions--------------------//
VOID EmulateFloatRegDifference(float *reg1Ptr, float *reg2Ptr, CONTEXT * ctxt, THREADID tid)
{
    
    // cout << "Emulating Float Dif in Regs" << endl;
    // cout << flush;
     
    // cout << " reg1: " << *reg1Ptr << " reg2: " << *reg2Ptr << " Dif res: " << (*reg1Ptr - (*reg2Ptr)) << endl;
    float temp = (*reg1Ptr); 
    *reg1Ptr = temp - (*reg2Ptr);

    unsigned char *c = reinterpret_cast<unsigned char*>(reg1Ptr);
    
    #ifdef STICK_ERROR_SOURCE
    unsigned char *store = reinterpret_cast<unsigned char*>(&temp);
    stickRandomBitsFloat(c, store);
    #endif
    //Add noise to all the lower 17 bits of a float value
    //Add 8 bits of noise anywhere
    #ifdef FLIPBIT_ERROR_SOURCE
    flipRandomBitsFloat(c);
    #endif
    
    #ifdef XOR_ERROR_SOURCE
    xor1BitsFloat(c);
    #endif
	SAxC_IC_COUNTS[SAxC_IC_TYPE_FLOAT_SUB_R_R]++;
    // cout << " reg1: " << *reg1Ptr << " reg2: " << *reg2Ptr << " Dif res: " << (*reg1Ptr - (*reg2Ptr)) << endl;

}

VOID EmulateFloatMemDifference(float *reg1Ptr, float *divisorPtr, unsigned int opSize, CONTEXT * ctxt, THREADID tid)
{
    
    // cout << "Emulating Float Dif in Mem" << endl;
    // cout << flush;
    float divisor = 0;
    PIN_SafeCopy(&divisor, divisorPtr, opSize);
     
    // cout << " reg1: " << *reg1Ptr << " mem: " << *divisorPtr /*<< " Dif res: " << (*reg1Ptr - (*reg2Ptr)) */<< endl;
    float temp = (*reg1Ptr); 
    *reg1Ptr = temp - divisor;

    unsigned char *c = reinterpret_cast<unsigned char*>(reg1Ptr);
    
    #ifdef STICK_ERROR_SOURCE
    unsigned char *store = reinterpret_cast<unsigned char*>(&temp);
    stickRandomBitsFloat(c, store);
    #endif
    //Add noise to all the lower 17 bits of a float value
    //Add 8 bits of noise anywhere
    #ifdef FLIPBIT_ERROR_SOURCE
    flipRandomBitsFloat(c);
    #endif
    
    #ifdef XOR_ERROR_SOURCE
    xor1BitsFloat(c);
    #endif
	SAxC_IC_COUNTS[SAxC_IC_TYPE_FLOAT_SUB_R_M]++;
    
    
    // cout << " reg1: " << *reg1Ptr << " mem: " << *divisorPtr /*<< " Dif res: " << (*reg1Ptr - (*reg2Ptr)) */<< endl;

}

VOID EmulateDoubleRegDifference(double *reg1Ptr, double *reg2Ptr, CONTEXT * ctxt, THREADID tid)
{
    
    // cout << "Emulating Double Dif in Regs" << endl;
    // cout << flush;
     
    // cout << " reg1: " << *reg1Ptr << " reg2: " << *reg2Ptr << " Dif res: " << (*reg1Ptr - (*reg2Ptr)) << endl;
    double temp = (*reg1Ptr); 
    *reg1Ptr = temp - (*reg2Ptr);

    unsigned char *c = reinterpret_cast<unsigned char*>(reg1Ptr);
    
    #ifdef STICK_ERROR_SOURCE
    unsigned char *store = reinterpret_cast<unsigned char*>(&temp);
    stickRandomBitsDouble(c, store);
    #endif
    //Add noise to all the lower 44 bits of a double value
    #ifdef FLIPBIT_ERROR_SOURCE
    flipRandomBitsDouble(c);
    #endif
    
    #ifdef XOR_ERROR_SOURCE
    xor1BitsDouble(c);
    #endif
	SAxC_IC_COUNTS[SAxC_IC_TYPE_DOUBLE_SUB_R_R]++;
    
    // cout << " reg1: " << *reg1Ptr << " reg2: " << *reg2Ptr << " Dif res: " << (*reg1Ptr - (*reg2Ptr)) << endl;

}

VOID EmulateDoubleMemDifference(double *reg1Ptr, double *divisorPtr, unsigned int opSize, CONTEXT * ctxt, THREADID tid)
{
    
    // cout << "Emulating Double Dif in Mem" << endl;
    // cout << flush;
    double divisor;
    PIN_SafeCopy(&divisor, divisorPtr, opSize);

    // cout << " reg1: " << *reg1Ptr << " mem: " << *divisorPtr /*<< " Dif res: " << (*reg1Ptr - (*reg2Ptr)) */<< endl;
    double temp = (*reg1Ptr); 
    *reg1Ptr = temp - divisor;

    unsigned char *c = reinterpret_cast<unsigned char*>(reg1Ptr);
      
    #ifdef STICK_ERROR_SOURCE
    unsigned char *store = reinterpret_cast<unsigned char*>(&temp);
    stickRandomBitsDouble(c, store);
    #endif  
    //Add 8 bits of noise anywhere
    #ifdef FLIPBIT_ERROR_SOURCE
    flipRandomBitsDouble(c);
    #endif
    
    #ifdef XOR_ERROR_SOURCE
    xor1BitsDouble(c);
    #endif
	SAxC_IC_COUNTS[SAxC_IC_TYPE_DOUBLE_SUB_R_M]++;
    
    // cout << " reg1: " << *reg1Ptr << " mem: " << *divisorPtr /*<< " Dif res: " << (*reg1Ptr - (*reg2Ptr)) */<< endl;

}

VOID EmulateIntDifference_R_R(int *reg1Ptr, int *reg2Ptr, CONTEXT * ctxt, THREADID tid)
{
    
    //cout << "Emulating Int Dif in Regs" << endl;
    // cout << flush;
     
    // cout << " reg1: " << *reg1Ptr << " reg2: " << *reg2Ptr << " Dif res: " << (*reg1Ptr - (*reg2Ptr)) << endl;
    int temp = (*reg1Ptr); 
    *reg1Ptr = temp - (*reg2Ptr);

    unsigned char *c = reinterpret_cast<unsigned char*>(reg1Ptr);
    
    #ifdef STICK_ERROR_SOURCE
    unsigned char *store = reinterpret_cast<unsigned char*>(&temp);
    stickRandomBitsInt(c, store);
    #endif
    //Add noise to all the lower 17 bits of a float value
    //Add 8 bits of noise anywhere
    #ifdef FLIPBIT_ERROR_SOURCE
    flipRandomBitsInt(c);
    #endif
    
    #ifdef XOR_ERROR_SOURCE
    xor1BitsInt(c);
    #endif
	SAxC_IC_COUNTS[SAxC_IC_TYPE_INT_SUB_R_R]++;
    // cout << " reg1: " << *reg1Ptr << " reg2: " << *reg2Ptr << " Dif res: " << (*reg1Ptr - (*reg2Ptr)) << endl;

}

VOID EmulateIntDifference_R_M(int *reg1Ptr, int *divisorPtr, unsigned int opSize, CONTEXT * ctxt, THREADID tid)
{
    
    //cout << "Emulating Int Dif in Mem" << endl;
    // cout << flush;
    int divisor = 0;
    PIN_SafeCopy(&divisor, divisorPtr, opSize);
     
    // cout << " reg1: " << *reg1Ptr << " mem: " << *divisorPtr /*<< " Dif res: " << (*reg1Ptr - (*reg2Ptr)) */<< endl;
    float temp = (*reg1Ptr); 
    *reg1Ptr = temp - divisor;

    unsigned char *c = reinterpret_cast<unsigned char*>(reg1Ptr);
    
    #ifdef STICK_ERROR_SOURCE
    unsigned char *store = reinterpret_cast<unsigned char*>(&temp);
    stickRandomBitsInt(c, store);
    #endif
    //Add noise to all the lower 17 bits of a float value
    //Add 8 bits of noise anywhere
    #ifdef FLIPBIT_ERROR_SOURCE
    flipRandomBitsInt(c);
    #endif
    
    #ifdef XOR_ERROR_SOURCE
    xor1BitsInt(c);
    #endif
	SAxC_IC_COUNTS[SAxC_IC_TYPE_INT_SUB_R_M]++;
    
    
    // cout << " reg1: " << *reg1Ptr << " mem: " << *divisorPtr /*<< " Dif res: " << (*reg1Ptr - (*reg2Ptr)) */<< endl;

}


VOID EmulateIntDifference_M_R(int *reg1Ptr, int *memPtr, unsigned int opSize, CONTEXT * ctxt, THREADID tid)
{
    
    //cout << "Emulating Int Dif in Mem" << endl;
    cout << flush;
    int mem = 0;
    PIN_SafeCopy(&mem, memPtr, opSize);
     
    //cout << " reg1: " << *reg1Ptr << " mem: " << mem /*<< " Dif res: " << (*reg1Ptr - (*reg2Ptr)) */<< endl;
    int temp = mem - *reg1Ptr;

    unsigned char *c = reinterpret_cast<unsigned char*>(&temp);
    
    #ifdef STICK_ERROR_SOURCE
    unsigned char *store = reinterpret_cast<unsigned char*>(&mem);
    stickRandomBitsInt(c, store);
    #endif
    //Add noise to all the lower 17 bits of a float value
    //Add 8 bits of noise anywhere
    #ifdef FLIPBIT_ERROR_SOURCE
    flipRandomBitsInt(c);
    #endif
    
    #ifdef XOR_ERROR_SOURCE
    xor1BitsInt(c);
    #endif
	SAxC_IC_COUNTS[SAxC_IC_TYPE_INT_SUB_R_M]++;
    
    PIN_SafeCopy(memPtr, &temp, opSize);
     
    
    //cout << " reg1: " << *reg1Ptr << " mem: " << *memPtr /*<< " Dif res: " << (*reg1Ptr - (*reg2Ptr)) */<< endl;

}

/* ===================================================================== */
/* Analysis routine that emulates Load/Store                             */
/* ===================================================================== */
// Move from memory to register
VOID DoLoad_MOVSS_XMM_MEM(float *regPtr, float * addr, REG regno)
{
    
    // cout << "BEFORE: DoLoad_MOVSS_XMM_MEM Emulate loading from addr " << addr << " to " << REG_StringShort(regno) << " current val " << *regPtr << endl;
    float value;
    PIN_SafeCopy(&value, addr, sizeof(float));
    // cout << "Load value " << value << endl;


    unsigned char *c = reinterpret_cast<unsigned char*>(&value);

    
    #ifdef STICK_ERROR_SOURCE
    unsigned char *store = reinterpret_cast<unsigned char*>(regPtr);
    stickRandomBitsFloat(c, store);
    #endif  
    //Add noise to all the lower 17 bits of a float value
    #ifdef FLIPBIT_ERROR_SOURCE
    flipRandomBitsFloat(c);
    #endif
    
    #ifdef XOR_ERROR_SOURCE
    xor1BitsFloat(c);
    #endif

    SAxC_IC_COUNTS[SAxC_IC_TYPE_FLOAT_MV_M_R]++;
    *regPtr = value;
    // cout << "AFTER : Emulate loading to addr " << addr << " from " << REG_StringShort(regno) << " current val " << *regPtr << endl;
    //return value;

}

// Move from register to memory
VOID DoLoad_MOVSS_MEM_XMM(float *regPtr, float * addr, REG regno)
{
    
    // cout << "BEFORE: DoLoad_MOVSS_MEM_XMM Emulate loading to addr " << addr << " from " << REG_StringShort(regno) << " current val " << *regPtr << endl;
    float value;
    PIN_SafeCopy(&value, regPtr, sizeof(float));

    unsigned char *c = reinterpret_cast<unsigned char*>(&value);
        
    #ifdef STICK_ERROR_SOURCE
    unsigned char *store = reinterpret_cast<unsigned char*>(addr);
    stickRandomBitsFloat(c, store);
    #endif
    //Add noise to all the lower 17 bits of a float value
    #ifdef FLIPBIT_ERROR_SOURCE
    flipRandomBitsFloat(c);
    #endif
    
    #ifdef XOR_ERROR_SOURCE
    xor1BitsFloat(c);
    #endif

    SAxC_IC_COUNTS[SAxC_IC_TYPE_FLOAT_MV_R_M]++;
    PIN_SafeCopy(addr, &value, sizeof(float));
    //(*addr) = value;
    // cout << "AFTER : Emulate loading to addr " << addr << " from " << REG_StringShort(regno) << " current val " << *regPtr << endl;
    //return value;

}

// Move from memory to register
VOID DoLoad_MOVSD_XMM_MEM(double *regPtr, double * addr, REG regno)
{
    
    // cout << "BEFORE: DoLoad_MOVSD_XMM_MEM Emulate loading from addr " << addr << " to " << REG_StringShort(regno) << " current val " << *regPtr << endl;
    double value;
    PIN_SafeCopy(&value, addr, sizeof(double));
    // cout << "Load value " << value << endl;

    unsigned char *c = reinterpret_cast<unsigned char*>(&value);
        
    #ifdef STICK_ERROR_SOURCE
    unsigned char *store = reinterpret_cast<unsigned char*>(regPtr);
    stickRandomBitsDouble(c, store);
    #endif
    //Add noise to all the lower 44 bits of a double value
    //Add 8 bits of noise anywhere
    #ifdef FLIPBIT_ERROR_SOURCE
    flipRandomBitsDouble(c);
    #endif
    
    #ifdef XOR_ERROR_SOURCE
    xor1BitsDouble(c);
    #endif

	SAxC_IC_COUNTS[SAxC_IC_TYPE_DOUBLE_MV_M_R]++;
    //printf("%17.15f\n", value);
    // cout << "Load value " << value << endl;
    *regPtr = value;
    // cout << "AFTER : Emulate loading from addr " << addr << " to " << REG_StringShort(regno) << " current val " << *regPtr << endl;
    //return value;

}

// Move from register to memory
VOID DoLoad_MOVSD_MEM_XMM(double *regPtr, double * addr, REG regno)
{
    
    // cout << "BEFORE: DoLoad_MOVSD_MEM_XMM Emulate loading to addr " << addr << " from " << REG_StringShort(regno) << " current val " << *regPtr << endl;
    double value;
    PIN_SafeCopy(&value, regPtr, sizeof(double));
    

    unsigned char *c = reinterpret_cast<unsigned char*>(&value);
      
    #ifdef STICK_ERROR_SOURCE
    unsigned char *store = reinterpret_cast<unsigned char*>(addr);
    stickRandomBitsDouble(c, store);
    #endif
      
    //Add noise to all the lower 44 bits of a double value
    //Add 8 bits of noise anywhere
    #ifdef FLIPBIT_ERROR_SOURCE
    flipRandomBitsDouble(c);
    #endif
    
    #ifdef XOR_ERROR_SOURCE
    xor1BitsDouble(c);
    #endif

    SAxC_IC_COUNTS[SAxC_IC_TYPE_DOUBLE_MV_R_M]++;
    PIN_SafeCopy(addr, &value, sizeof(double));
    //(*addr) = value;
    // cout << "AFTER : Emulate loading to addr " << addr << " from " << REG_StringShort(regno) << " current val " << *regPtr << endl;
    //return value;

}


// Move from register to memory
VOID DoLoad_MOV_GPRv_MEMv(int *regPtr, int * addr, REG regno)
{
    //cout << "Load Int: Mem->GPR"<<endl;
    //cout << "BEFORE: DoLoad_MOVSD_XMM_MEM Emulate loading from addr " << addr << " to " << REG_StringShort(regno) << " current val " << *regPtr << endl;
    int value;
    PIN_SafeCopy(&value, addr, sizeof(int));
    //cout << "Load value " << value << endl;

    unsigned char *c = reinterpret_cast<unsigned char*>(&value);
        
    #ifdef STICK_ERROR_SOURCE
    unsigned char *store = reinterpret_cast<unsigned char*>(regPtr);
    stickRandomBitsInt(c, store);
    #endif
    //Add noise to all the lower 44 bits of a double value
    //Add 8 bits of noise anywhere
    #ifdef FLIPBIT_ERROR_SOURCE
    flipRandomBitsInt(c);
    #endif
    
    #ifdef XOR_ERROR_SOURCE
    xor1BitsInt(c);
    #endif

	SAxC_IC_COUNTS[SAxC_IC_TYPE_INT_MV_M_R]++;
    //printf("%17.15f\n", value);
    // cout << "Load value " << value << endl;
    *regPtr = value;
    //cout << "AFTER : Emulate loading from addr " << addr << " to " << REG_StringShort(regno) << " current val " << *regPtr << endl;
    //return value;

}

// Move from register to memory
VOID DoLoad_MOV_MEMv_GPRv(int *regPtr, int * addr, REG regno)
{
    //cout << "Load Int: GPR->Mem"<<endl;
    //cout << "BEFORE: DoLoad_MOVSD_MEM_XMM Emulate loading to addr " << addr << " from " << REG_StringShort(regno) << " current val " << *regPtr << endl;
    int value;
    PIN_SafeCopy(&value, regPtr, sizeof(int));
    //cout << "Load value " << value << endl;


    unsigned char *c = reinterpret_cast<unsigned char*>(&value);
      
    #ifdef STICK_ERROR_SOURCE
    unsigned char *store = reinterpret_cast<unsigned char*>(addr);
    stickRandomBitsInt(c, store);
    #endif
      
    //Add noise to all the lower 44 bits of a double value
    //Add 8 bits of noise anywhere
    #ifdef FLIPBIT_ERROR_SOURCE
    flipRandomBitsInt(c);
    #endif
    
    #ifdef XOR_ERROR_SOURCE
    xor1BitsInt(c);
    #endif

    SAxC_IC_COUNTS[SAxC_IC_TYPE_INT_MV_R_M]++;
    PIN_SafeCopy(addr, &value, sizeof(int));
    //(*addr) = value;
    //cout << "AFTER : Emulate loading to addr " << addr << " from " << REG_StringShort(regno) << " current val " << *regPtr << endl;
    //return value;

}
//All info about an instruction
VOID InstructionDecodeFunc(INS ins)
{

    // cout << "Instruction is " << INS_Mnemonic(ins) << endl;
    // cout << flush;
    //------------------//
    xed_decoded_inst_t* xedd = INS_XedDec(ins);
    
    
    xed_iform_enum_t iform = xed_decoded_inst_get_iform_enum(xedd);
    string string_iform = xed_iform_enum_t2str(iform);
    // cout << "IFORM is " << string_iform << endl;

    // To print out the gory details uncomment this:
    char buf[2048];
    xed_decoded_inst_dump(xedd, buf, 2048);
    // cout << buf << endl;
    
    xed_syntax_enum_t syntax = XED_SYNTAX_INTEL;  // XED_SYNTAX_ATT, XED_SYNTAX_XED
    const UINT32 BUFLEN = 100;
    char buffer[BUFLEN];
    ADDRINT addr = INS_Address(ins);
    BOOL ok = xed_format(syntax, xedd, buffer, BUFLEN, static_cast<UINT64>(addr));
    if (ok)
    {
         cout << setw(sizeof(ADDRINT)*2) 
             << hex << addr << dec << " "  << buffer << endl;
    }
    else
    {
         cout << "disas-error @" << hex << addr << dec << "\n" << endl;
    }
    //------------------//

}

