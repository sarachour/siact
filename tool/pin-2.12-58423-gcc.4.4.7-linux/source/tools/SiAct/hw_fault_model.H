#ifndef HW_FAULT_MODEL
#define HW_FAULT_MODEL

#include "pin.H"
#include "pin_profile.H"
#include <vector>
#include "fault_const.H"
#include "xorshift.H"
#include <map>
#include "cache.H"
#include "approx.H"

#include "hierarchy.H"

#define USE_HARDWARE_FAULT_MODEL
/*
 * 
 * DRAM refresh: per second bit flip probability:
10^-9   10^-5   10^-3

Memory power saved:
17      22      24

SRAM read/write upset probability
read: 10^-16.7  10-7.4  10^-3
write: 10^-5.59 10^-4.94        10^-3

Supply Power Saved:
70      80      90

float mantissa bits:
16      8       4

double mantissa bits:
32      16      8

energy saved per-op:
32      78      85

Arithmetic timing error probability:
10-6    10^-4   10^-2

Energy Saved Per operation
12      22      30

 */


typedef struct HW_MODEL_INFO_T {
	int idx;
	uint32_t srand;
	Tuple<PreciseHierarchy,ApproximateHierarchy>* mem;
} hw_info_t;


std::vector<hw_info_t> hw_model_list;


hw_info_t model;
bool loaded = false;

KNOB<string> KnobHardwareSpecFile(KNOB_MODE_WRITEONCE, "pintool",
  "hwmodel", "hardwareModel.txt", "specify mappings to a hardware model");


KNOB<INT> KnobGlobalSeed (KNOB_MODE_WRITEONCE, "pintool", "s", "0", "Set global seed");

void print_hardware_fault_model(FILE * out){
  fprintf(out, "Hardware Fault Model Info\n");
  fprintf(out, "INDEX: %d\nSEED: %d\n", model.idx, model.srand);
  if(model.mem->hasFirst()) model.mem->first()->description(out); 
  else model.mem->second()->description(out);
	fprintf(out, "------------------");
}

VOID setRGENSeed(){
  sxorshift32(model.srand);

}

VOID load_hardware_fault_model(ADDRINT model_ptr){
  unsigned int i;
  int model_index = model_ptr;
  loaded = true;
  for(i=0; i < hw_model_list.size(); i++){
    hw_info_t hinfo = hw_model_list[i];
    if(hinfo.idx == model_index){
      printf("[HARDWARE] LOADING FAULT MODEL %d\n", i);
      model = hinfo;
      setRGENSeed();
      memory = model.mem;
      print_hardware_fault_model(stdout);
    }
  }

}

#define streq(a,b) (strcmp(a,b) == 0)
void readHardwareFaultModelFile(){
  std::ifstream myReadFile;
  cout << "[HARDWARE] LOADING FAULT MODEL FROM FILE: "  << KnobHardwareSpecFile.Value().c_str() << endl;
  myReadFile.open(KnobHardwareSpecFile.Value().c_str());
  if(myReadFile){
    string output;
    while (std::getline(myReadFile, output))
        {
            //cout << "Read line                  : " << output << endl;
          if(output.find("#") != string::npos)
            {
                cout << "[HARDWARE] Ignoring comment line:       " << output << "\n";
            }
            else //if "#" is not found in the string
            {
              char sramtype[128];
              char dramtype[128];
              char overalltype[128];
              UINT32 l1i_cache, l1i_line, l1i_assoc; //cache is in kb
              UINT32 l1d_cache, l1d_line, l1d_assoc;
              UINT32 l2_cache, l2_line, l2_assoc;
              ApproximateCacheModel l1dmodel;
              ApproximateCacheModel l2model;
              ApproximateMemoryModel memmodel;
              int idx;
              uint32_t seed;
              hw_info_t templ;
              //1 bit-flip-prob=0.1 err-bits=9
              sscanf(output.c_str(), "%d seed=%d type=%s cache-type=%s mem-type=%s l1i=%d,%d,%d l1d=%d,%d,%d l2=%d,%d,%d", 
				&idx, &seed,
				overalltype, sramtype, dramtype, 
				&l1i_cache, &l1i_line,&l1i_assoc,
				&l1d_cache, &l1d_line, &l1d_assoc,
				&l2_cache, &l2_line, &l2_assoc);
              
              templ.srand = KnobGlobalSeed.Value() + seed;
              templ.idx = idx;
              if(streq(sramtype, "medium")){
				  l1dmodel = l2model = CacheModelMedium;
			  }
			  else if(streq(sramtype, "heavy")){
				  l1dmodel = l2model = CacheModelHeavy;
			  }
			  else{
				  l1dmodel = l2model = CacheModelNone;
			  }
			  if(streq(dramtype, "static")){
				  memmodel = MemoryModelStatic;
			  }
			  else if(streq(dramtype, "dynamic")){
				  memmodel = MemoryModelDynamic;
			  }
			  else if(streq(dramtype, "variant")){
				  memmodel = MemoryModelVariation;
			  }
			  else if(streq(dramtype, "temp")){
				  memmodel = MemoryModelTemp;
			  }
			  else {
				  memmodel = MemoryModelNone;
			  }
              if(streq(overalltype, "precise")){
				  templ.mem = new Tuple<PreciseHierarchy,ApproximateHierarchy>(new PreciseHierarchy(
					l1i_cache*KILO, l1i_line, l1i_assoc,
					l1d_cache*KILO, l1d_line, l1d_assoc,
					l2_cache*KILO, l2_line, l2_assoc));
			  }
			  else if(streq(overalltype, "approximate")){
				   templ.mem = new Tuple<PreciseHierarchy,ApproximateHierarchy>(
						new ApproximateHierarchy(
						l1i_cache*KILO, l1i_line, l1i_assoc,
						l1d_cache*KILO, l1d_line, l1d_assoc,
						l2_cache*KILO, l2_line, l2_assoc,
						l1dmodel, l2model, memmodel));
			  }
              hw_model_list.push_back(templ);

            }
        }
        myReadFile.close();
  }
  else{
    cout << "[HARDWARE] NO EXISTING FAULT MODEL. USING A NO-ERROR APPROACH" << endl;
  }
}
int getHWModelIDX(){
	if(loaded)
		return model.idx;
	else
		return 9999;
}

#endif
