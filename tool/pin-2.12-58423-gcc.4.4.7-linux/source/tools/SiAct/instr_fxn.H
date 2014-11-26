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


bool is_task_begin(string functionName){
	string libFxnName = "PIN_START_TASK";
	
	if( ( libFxnName == functionName ) )
	{
		cout << "[PIN] Found Start Task:  " << functionName << endl;
		return(true);
	}
	else 
		return(false);
}

bool is_task_end(string functionName){
	string libFxnName = "PIN_STOP_TASK";
	
	if( ( libFxnName == functionName ) )
	{
		cout << "[PIN] Found Stop Task:  " << functionName << endl;
		return(true);
	}
	else 
		return(false);
}
bool is_refresh_dram(string functionName){	
	string libFxnName = "PIN_REFRESH_DRAM";
	
	if( ( libFxnName == functionName ) )
	{
		cout << "[PIN] Found DRAM Refresh function:  " << functionName << endl;
		return(true);
	}
	else 
		return(false);
}
bool is_hw_bind(string functionName){	
	string libFxnName = "PIN_BIND_MODEL";
	
	if( ( libFxnName == functionName ) )
	{
		cout << "[PIN] Found HW Function:  " << functionName << endl;
		return(true);
	}
	else 
		return(false);
}

bool is_timer_start(string functionName){
	string libFxnName = "PIN_START_TIMER";
	
	if( ( libFxnName == functionName ) )
	{
		cout << "[PIN] Found Start Timer Function:  " << functionName << endl;
		return(true);
	}
	else 
		return(false);	
}

bool is_timer_end(string functionName){
	string libFxnName = "PIN_STOP_TIMER";
	
	if( ( libFxnName == functionName ) )
	{
		cout << "[PIN] Found Stop Timer Function:  " << functionName << endl;
		return(true);
	}
	else 
		return(false);	
}
bool is_start_inject(string functionName){
	string libFxnName = "PIN_START_INJECT_ERRORS";
	
	if( ( libFxnName == functionName ) )
	{
		cout << "[PIN] Found Start Inject Errors Function:  " << functionName << endl;
		return(true);
	}
	else 
		return(false);	
}
bool is_stop_inject(string functionName){
	string libFxnName = "PIN_STOP_INJECT_ERRORS";
	
	if( ( libFxnName == functionName ) )
	{
		cout << "[PIN] Found Stop Inject Errors Function:  " << functionName << endl;
		return(true);
	}
	else 
		return(false);	
}


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
	PIN_LISTED_FUNCTION
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
		else if(is_in_function_list(functionName))
			return PIN_LISTED_FUNCTION;
		else
			return PIN_NORMAL;
	
}
