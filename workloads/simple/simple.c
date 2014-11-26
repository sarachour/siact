
#include "stdio.h"

double simple(float a, float b, double c, double d){

	a=a/b;
	c=c/d;
	b=a+b;
	d=d+c;
	a=a-b;
	c=c-d;

}
int simple3(){
	int a = 4;
	printf("%d\n", a);
}
int simple2(int i, int j){
		int sum=0, diff=0, prod=1;
		j=2;
		int emm;
		emm = 100;
		while(emm > 0){
			diff -= emm;
			prod *= emm;
			sum += emm;
			emm--;
		}
		printf("sum: %d\n diff %d\n prod %d\n", sum, diff, prod);
		return i;
}
int main(){
	float a=1.0;
	float b=2.0;
	double c=3.0;
	double d=4.0;
	int i=0;
	int j=5;
	j = simple2(i,j);
	printf("end: %d\n", j);
}
