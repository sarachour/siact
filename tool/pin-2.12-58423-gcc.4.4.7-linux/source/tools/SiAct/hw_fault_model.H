#ifndef HW_FAULT_MODEL
#define HW_FAULT_MODEL

#include "pin.H"
#include "pin_profile.H"
#include <vector>
#include "fault_const.H"
#include "xorshift.h"
#include <map>
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


typedef struct HW_MODEL_INFO_T{
	float sram_read_err_int;
	float sram_write_err_int;
	float sram_read_err_fp;
	float sram_write_err_fp;
	uint32_t dbl_err_nbits;
	uint32_t flt_err_nbits;
	float dram_per_sec_bitflip_int;
	float dram_per_sec_bitflip_fp;
	uint32_t SRAM_READ_ERR_INT;
	uint32_t SRAM_WRITE_ERR_INT;
	uint32_t SRAM_READ_ERR_FP;
	uint32_t SRAM_WRITE_ERR_FP;
	uint32_t DRAM_PER_SEC_BITFLIP_INT;
	uint32_t DRAM_PER_SEC_BITFLIP_FP;
	uint8_t DRAM_ENABLE;
	uint32_t dbl_err_mask;
	uint32_t flt_err_mask;
	//index
	int idx;
	uint32_t srand;
} hw_info_t;


std::map<std::string, hw_info_t> hw_models;

uint32_t create_mask(int n){
	uint32_t mask = 0;
	for(int i=0; i < n ; i++){
		mask = (0x1<<i) | mask;
	}
	return mask;
}
#define RGEN_MAX UINT_MAX
hw_info_t create_spec(float sram_rd_int, float sram_wr_int, float sram_rd_fp, float sram_wr_fp, 
					float dram_bf_int, float dram_bf_fp,
					int dbl_err_nbits, int flt_err_nbits){
	hw_info_t t;
	t.sram_read_err_fp = sram_rd_fp;
	t.sram_read_err_int = sram_rd_int;
	t.sram_write_err_fp = sram_wr_fp;
	t.sram_write_err_int = sram_wr_int;
	t.dram_per_sec_bitflip_fp = dram_bf_fp;
	t.dram_per_sec_bitflip_int = dram_bf_int;
	t.dbl_err_nbits = dbl_err_nbits;
	t.flt_err_nbits = flt_err_nbits;
	t.dbl_err_mask = create_mask(t.dbl_err_nbits);
	t.flt_err_mask = create_mask(t.flt_err_nbits);
	t.SRAM_READ_ERR_FP = t.sram_read_err_fp*RGEN_MAX;
	t.SRAM_WRITE_ERR_FP = t.sram_write_err_fp*RGEN_MAX;
	t.SRAM_READ_ERR_INT = t.sram_read_err_int*RGEN_MAX;
	t.SRAM_WRITE_ERR_INT =  t.sram_write_err_int*RGEN_MAX;
	t.DRAM_PER_SEC_BITFLIP_FP = t.dram_per_sec_bitflip_fp*RGEN_MAX;
	t.DRAM_PER_SEC_BITFLIP_INT = t.dram_per_sec_bitflip_int*RGEN_MAX;
	if(t.DRAM_PER_SEC_BITFLIP_FP > 0 || t.DRAM_PER_SEC_BITFLIP_INT > 0)
		t.DRAM_ENABLE = 1;
	else
		t.DRAM_ENABLE = 0;
	return t;
}
void _createHWMap(int use_integer_approx, int use_reduced_mantissa, int use_drams){
	//dbl = 52 bits
	//flt = 23 bits
	char name[128];
	int u_ia = use_integer_approx; //use integer approximation
	int u_rm = use_reduced_mantissa; //use reduced mantissa
	int u_d = use_drams;
	hw_info_t aggressive = create_spec(
		pow(10,-3)*u_ia, pow(10,-3)*u_ia, pow(10,-3), pow(10,-3), //sram
		pow(10,-3)*u_ia*u_d, pow(10,-3)*u_d, //dram_refresh
		(52-8)*u_rm, (23-4)*u_rm
	);
	hw_info_t heavy = create_spec(
		pow(10,-5)*u_ia, pow(10,-4)*u_ia,pow(10,-5), pow(10,-4), //sram
		pow(10,-4)*u_ia*u_d, pow(10,-4)*u_d, //dram_refresh
		(52-12)*u_rm, (23-6)*u_rm
	);
	hw_info_t heavymed = create_spec(
		pow(10,-6.2)*u_ia, pow(10,-4.5)*u_ia,pow(10,-6.2), pow(10,-4.5), //sram
		pow(10,-4.5)*u_ia*u_d, pow(10,-4.5)*u_d, //dram_refresh
		(52-14)*u_rm, (23-7)*u_rm
	);
	hw_info_t none = create_spec(
		0,0,0,0, //sram
		pow(10,-4.5)*u_ia*u_d, pow(10,-4.5)*u_d, //dram_refresh
		0,0
	);
	hw_info_t medium = create_spec(
		pow(10,-7.4)*u_ia, pow(10,-4.94)*u_ia, pow(10,-7.4), pow(10,-4.94), //sram
		pow(10,-5)*u_ia*u_d, pow(10,-5)*u_d, //dram_refresh
		(52-16)*u_rm, (23-8)*u_rm
	);
	hw_info_t mild = create_spec(
		pow(10,-16.7)*u_ia, pow(10,-5.59)*u_ia, pow(10,-16.7), pow(10,-5.59), //sram
		pow(10,-9)*u_ia*u_d, pow(10,-9)*u_d, //dram_refresh
		(52-32)*u_rm, (23-16)*u_rm
	);
	sprintf(name,"%s%s%s%s", "heavy", u_ia == 1 ? ".int":"", u_d == 1 ? ".dram" : "", u_rm == 1? ".mant" : "");
	hw_models[name] = heavy;
	sprintf(name,"%s%s%s%s", "heavymed", u_ia == 1 ? ".int":"", u_d == 1 ? ".dram" : "", u_rm == 1? ".mant" : "");
	hw_models[name] = heavymed;
	sprintf(name,"%s%s%s%s", "aggressive", u_ia == 1 ? ".int":"", u_d == 1 ? ".dram" : "", u_rm == 1? ".mant" : "");
	hw_models[name] = aggressive;
	sprintf(name,"%s%s%s%s", "medium", u_ia == 1 ? ".int":"", u_d == 1 ? ".dram" : "", u_rm == 1? ".mant" : "");
	hw_models[name] = medium;
	sprintf(name,"%s%s%s%s", "mild", u_ia == 1 ? ".int":"", u_d == 1 ? ".dram" : "", u_rm == 1? ".mant" : "");
	hw_models[name] = mild;
	sprintf(name,"%s%s%s%s", "none", u_ia == 1 ? ".int":"", u_d == 1 ? ".dram" : "", u_rm == 1? ".mant" : "");
	hw_models[name] = none;
}
void createHWMap(){
	hw_info_t perfect = create_spec(
		0,0,0,0, //sram
		0,0,
		0,0
	);
	_createHWMap(0,0,0);
	_createHWMap(1,0,0);
	_createHWMap(0,1,0);
	_createHWMap(0,0,1);
	_createHWMap(0,1,1);
	_createHWMap(1,1,0);
	_createHWMap(1,0,1);
	_createHWMap(1,1,1);
	hw_models["perfect"] = perfect;
}

std::vector<hw_info_t> hw_model_list;


hw_info_t model;
bool loaded = false;

KNOB<string> KnobHardwareSpecFile(KNOB_MODE_WRITEONCE, "pintool",
  "hwmodel", "hardwareModel.txt", "specify mappings to a hardware model");


KNOB<INT> KnobGlobalSeed (KNOB_MODE_WRITEONCE, "pintool", "s", "0", "Set global seed");

void print_hardware_fault_model(FILE * out){
  fprintf(out, "Hardware Fault Model Info\n");
  fprintf(out, "idx: %d, seed: %d\n", model.idx, model.srand);
  fprintf(out, "sram	read	write\n");
  fprintf(out, "fp	%e	%e\n", model.sram_read_err_fp, model.sram_write_err_fp);
  fprintf(out, "int	%e	%e\n\n", model.sram_read_err_int, model.sram_write_err_int);
  fprintf(out, "dram	perr/sec\n");
  fprintf(out, "fp	%e\n", model.dram_per_sec_bitflip_fp);
  fprintf(out, "int	%e\n\n", model.dram_per_sec_bitflip_int);
  fprintf(out, "mant-type	n-nonsig-bits	mask\n");
  fprintf(out, "fp-mant:	%d %x\n", model.flt_err_nbits, model.flt_err_mask);
  fprintf(out, "dbl-mant:	%d %x\n", model.dbl_err_nbits, model.dbl_err_mask);

}

VOID __setHardwareFaultModel(){
  sxorshift32(model.srand);

}

VOID loadHardwareFaultModel(ADDRINT model_ptr){
  unsigned int i;
  int model_index = model_ptr;
  loaded = true;
  for(i=0; i < hw_model_list.size(); i++){
    hw_info_t hinfo = hw_model_list[i];
    if(hinfo.idx == model_index){
      printf("[HARDWARE] LOADING FAULT MODEL %d\n", i);
      model = hinfo;
      __setHardwareFaultModel();
      print_hardware_fault_model(stdout);
    }
  }

}

void readHardwareFaultModelFile(){
  std::ifstream myReadFile;
  cout << "[HARDWARE] LOADING MODEL DESCRIPTIONS\n"<<endl;
  createHWMap();
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
              char name[128];
              int idx;
              uint32_t seed;
              //1 bit-flip-prob=0.1 err-bits=9
              sscanf(output.c_str(), "%d	name=%s	seed=%d", &idx, name, &seed);
              hw_info_t templ = hw_models[name];
              templ.srand = KnobGlobalSeed.Value() + seed;
              templ.idx = idx;
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
