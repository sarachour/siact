#include "pin_util.h"

float boo[150];
void app1(){
	float fbar;
	float foot=0;
	float sum = 0;
	float buf[100];
	float total = 0;
	float n =0;
	fbar = 1.0*15-3;
	foot = 15;
	boo[10] = fbar*10;
	pin_mark_urel(boo, sizeof(float)*150);
	pin_mark_urel(buf, sizeof(float)*150);
	pin_mark_urel(&fbar, sizeof(float));
	pin_mark_urel(&sum, sizeof(float));
	pin_mark_urel(&foot, sizeof(float));
	printf("starting loop\n");
	for(int i=0; i < 10000; i++){
		sum = 0;
		for(int j = 0; j < 100; j++){
			buf[j] = j*foot;
			sum += buf[j];
			total += boo[10];
			n++;
			printf("%d\n", j);
		}
		if(i%100 == 0) printf("%f %f\n", sum,boo[10]);
	}
	
	printf("%f %f %f\n", total/n, sum,boo[10]);
	
}

int main(){
	PIN_BIND_MODEL(1);
	printf("# START TASK\n");
	pin_start_inject_errors();
	app1();
	pin_start_inject_errors();
	printf("# END TASK\n");
}
