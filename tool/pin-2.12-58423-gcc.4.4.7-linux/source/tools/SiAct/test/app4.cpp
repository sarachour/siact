#include "pin_util.h"

void app1(){
	int fbar = 15;
	pin_mark_urel((uint64_t) &fbar, sizeof(int));
	
	printf("STARTfbar: %d %lx\n", fbar, (uint64_t) &fbar);
	for(int i=0; i < 1000000; i++){
		fbar = fbar - 1;
		fbar++;
		//printf("fbar: %f\n", fbar);
	}
	printf("END fbar: %d\n", fbar);
	
}

int main(){
	PIN_BIND_MODEL(1);
	printf("# START\n");
	pin_start_inject_errors();
	app1();
	pin_start_inject_errors();
	printf("# END\n");
}
