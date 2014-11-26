#include "pin_util.h"
#include "stdio.h"

void app1(){
	float fbar;
	float foot=0;
	fbar = 1.0*15-3;
	for(int i=0; i < 10000; i++){
		foot += fbar/i;
	}
	
}

void app2(){
	printf("moomoo\n");
	
}

int main(){
	long int tims[3];
	PIN_BIND_MODEL(1);
	PIN_START_TIMER(0);
	PIN_START_TIMER(1);
	app1();
	PIN_STOP_TIMER(1,&tims[1]);
	PIN_START_TIMER(2);
	app2();
	PIN_STOP_TIMER(2,&tims[2]);
	PIN_STOP_TIMER(0,&tims[0]);
	printf("Timer 0:%ld\n", tims[0]);
	printf("Timer 1:%ld\n", tims[1]);
	printf("Timer 2:%ld\n", tims[2]);
	PIN_START_TIMER(0);
	PIN_START_TIMER(1);
	app1();
	PIN_STOP_TIMER(1,&tims[1]);
	PIN_START_TIMER(2);
	app2();
	PIN_STOP_TIMER(2,&tims[2]);
	PIN_STOP_TIMER(0,&tims[0]);
	printf("Timer 0:%ld\n", tims[0]);
	printf("Timer 1:%ld\n", tims[1]);
	printf("Timer 2:%ld\n", tims[2]);
}
