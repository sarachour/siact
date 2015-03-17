#include <set>


std::set<string> function_list;

KNOB<string> KnobFunctionList(KNOB_MODE_WRITEONCE,    "pintool", "t", "trace.config", "specify output file name");
    
void read_function_list()
{                                                                          
    
    std::ifstream myReadFile;                   
    myReadFile.open(KnobFunctionList.Value().c_str());           
    
    if(myReadFile)
    {
        string output;
        
        while (myReadFile >> output)
        {
            //cout << "Read line                  : " << output << endl;
        	if(output.find("#") != string::npos)
            {
                cout << "[PIN] Ignoring comment line:       " << output << endl;
            }
            else //if "#" is not found in the string
            {
                cout << "[PIN] Adding Function to Arch: " << output << endl;
        	    function_list.insert(output);
            }
        }

        //Print this data here since this probability makes sense only if AxC knob is specified
    }
    else
    {
        //Couldnt open the file
        cout << "[PIN] No file specified or file could not be opened" << endl;
    }
    
    myReadFile.close();

}   

bool is_in_function_list(string functionName)
{
    
    if( ( function_list.find(functionName) != function_list.end() ) )
    {   
        //Full string match - finds "functionName" in the "List"
	    cout << "[PIN] Found Targetted Function:  " << functionName << endl;
	    return(true);
    }
    return false;
    //else
	//    return(false);

}

#define test_name(fxn,name, msg) \
		if( name == fxn) { \
			cout << "[PIN] " << msg <<": "<<name<<endl; \
			return true; \
		} \
		else return false;
		
bool is_task_begin(string functionName){ test_name(functionName, "PIN_START_TASK", "Found Start Task"); }
bool is_task_end(string functionName){ test_name(functionName, "PIN_STOP_TASK", "Found Stop Task");}
bool is_refresh_dram(string functionName){	test_name(functionName, "PIN_REFRESH_DRAM", "Found DRAM Refresh Function");}
bool is_hw_bind(string functionName){	test_name(functionName, "PIN_BIND_MODEL", "Found Hardware Binding Function");}
bool is_timer_start(string functionName){test_name(functionName, "PIN_START_TIMER", "Found Start Timer Function");}
bool is_timer_end(string functionName){test_name(functionName, "PIN_STOP_TIMER", "Found Stop Timer Function");}
bool is_start_inject(string functionName){test_name(functionName, "PIN_START_INJECT_ERRORS", "Found Injection Starting Function");}
bool is_stop_inject(string functionName){test_name(functionName, "PIN_STOP_INJECT_ERRORS", "Found Injection Stopping Function");}
bool is_urel_alloc(string functionName){test_name(functionName, "PIN_UREL_MALLOC", "Found Unreliable Malloc Function");}

typedef enum FUNCTION_TYPE_T {
	PIN_STOP_TIMER,
	PIN_START_TIMER,
	PIN_BIND_FXN,
	PIN_START_TASK,
	PIN_STOP_TASK,
	PIN_START_INJECT_ERRORS,
	PIN_STOP_INJECT_ERRORS,
	PIN_NORMAL,
	PIN_DRAM_REFRESH,
	PIN_LISTED_FUNCTION,
	PIN_ALLOC_UREL
} function_type_t;

function_type_t get_function_type(string functionName){
		if(is_timer_end(functionName))
			return PIN_STOP_TIMER;
		else if(is_timer_start(functionName))
			return PIN_START_TIMER;
		else if(is_task_begin(functionName))
			return PIN_START_TASK;
		else if(is_task_end(functionName))
			return PIN_STOP_TASK;
		else if(is_start_inject(functionName))
			return PIN_START_INJECT_ERRORS;
		else if(is_stop_inject(functionName))
			return PIN_STOP_INJECT_ERRORS;
		else if(is_hw_bind(functionName))
			return PIN_BIND_FXN;
		else if(is_refresh_dram(functionName))
			return PIN_DRAM_REFRESH;
		else if(is_urel_alloc(functionName))
			return PIN_ALLOC_UREL;
		else if(is_in_function_list(functionName))
			return PIN_LISTED_FUNCTION;
		else
			return PIN_NORMAL;
	
}
