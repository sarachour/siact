#include "pin_util.h"

void app1(){
	float fbar;
	float foot=0;
	fbar = 1.0*15-3;
	pin_mark_urel((uint64_t) &fbar, sizeof(float));
	pin_mark_urel((uint64_t) &foot, sizeof(float));
	printf("inapp: %lx\n", (uint64_t) &fbar);
	printf("inapp: %lx\n", (uint64_t) &foot);
	for(int i=0; i < 100000; i++){
		foot += fbar/i;
	}
	
}

int main(){
	PIN_BIND_MODEL(1);
	printf("# START\n");
	pin_start_inject_errors();
	app1();
	pin_start_inject_errors();
	printf("# END\n");
}
