#include "pin_util.h"

void app1(){
	float fbar;
	float foot=0;
	fbar = 1.0*15-3;
	for(int i=0; i < 10000; i++){
		foot += fbar/i;
	}
	
}

int main(){
	PIN_BIND_MODEL(1);
	app1();
	
}
