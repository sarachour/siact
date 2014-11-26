#ifndef HW_FAULT_MODEL
#define HW_FAULT_MODEL

#include "pin.H"
#include "pin_profile.H"
#include <vector>
#include "fault_const.H"
#define USE_HARDWARE_FAULT_MODEL



typedef struct HW_MODEL_INFO{
	float p_error;
	int err_bits;
	int idx;
} hw_model_info;

std::vector<hw_model_info> hw_model_list;
hw_model_info curr_hw_model;

KNOB<string> KnobHardwareSpecFile(KNOB_MODE_WRITEONCE, "pintool",
	"hwmodel", "hardwareModel.txt", "specify mappings to a hardware model");



bool isHWSpecFunction(string functionName){
	string HWSpecFunctionName = "__PIN_INSTRUMENT_HARDWARE_FAULT";
	if( ( HWSpecFunctionName == functionName ) )
    {   
        //Full string match - finds "functionName" in the "List"
	    cout << "[HARDWARE] Found HW Function:  " << functionName << endl;
	    
	    return(true);
    }
    else
		return(false);
}

void printHardwareFaultModel(std::ofstream& out){
	out << "Hardware Fault Model Info" << endl;
	out << "Index: " << curr_hw_model.idx << endl;
	out << "P(Error)" << curr_hw_model.p_error << endl;
	out << "Error Bits:" << curr_hw_model.err_bits << endl;
	out << endl;
}
VOID __setHardwareFaultModel(){
	FAULT_P_BITFLIP_ERROR = curr_hw_model.p_error;
	FAULT_N_FLIP_BITS = curr_hw_model.err_bits;
}
VOID loadHardwareFaultModel(ADDRINT model_ptr){
	unsigned int i;
	int model_index = model_ptr;
	for(i=0; i < hw_model_list.size(); i++){
		hw_model_info hinfo = hw_model_list[i];
		if(hinfo.idx == model_index){
			curr_hw_model = hinfo;
			__setHardwareFaultModel();
		}
	}
	
}

void readHardwareFaultModelFile(){
	std::ifstream myReadFile;   
	cout << "[HARDWARE] LOADING FAULT MODEL"  << KnobHardwareSpecFile.Value().c_str() << endl;    
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
				hw_model_info hinfo;
				//1 bit-flip-prob=0.1 err-bits=9
				sscanf(output.c_str(), "%d bit-flip-prob=%f err-bits=%d", &hinfo.idx, &hinfo.p_error, &hinfo.err_bits);
				hw_model_list.push_back(hinfo);
				
            }
        }
        myReadFile.close();
	}
	else{
		cout << "[HARDWARE] NO EXISTING FAULT MODEL. USING A NO-ERROR APPROACH" << endl; 
	}
}
int getHWModelIDX(){
	return curr_hw_model.idx;
}

#endif
