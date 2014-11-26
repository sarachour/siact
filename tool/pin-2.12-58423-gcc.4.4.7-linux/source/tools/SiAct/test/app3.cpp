#include "pin_util.h"
#include "stdio.h"


double app_A(){
	float fbar;
	float foot=0;
	fbar = 1.0*15-3;
	for(int i=0; i < 100000; i++){
		foot += fbar/i;
	}
	return foot;
	
}
double app_B(){
	app_A();
	app_A();
}
int main(){
	pin_task_info_t inf;
	pin_timer_info_t tim;
	PIN_BIND_MODEL(1);
	pin_start_timer(0);
	pin_start_task(0);
	app_A();
	pin_stop_task(0,&inf);
	pin_stop_timer(0, &tim);
	pin_print_task(stdout, "[TASK0/TEST1]", &inf); printf("\n");
	pin_print_timer(stdout, "TIMER0", &tim); printf("\n");
	pin_start_timer(0);
	pin_start_task(0);
	app_B();
	pin_stop_task(0,&inf);
	pin_stop_timer(0, &tim);
	pin_print_task(stdout, "[TASK0/TEST1]", &inf); printf("\n");
	pin_print_timer(stdout, "TIMER0", &tim); printf("\n");
}
