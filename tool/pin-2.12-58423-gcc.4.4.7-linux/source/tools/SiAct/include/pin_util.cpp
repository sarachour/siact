#include "pin_util.h"
#include "stdio.h"
#include "stdint.h"
//bind the hardware model
void PIN_REFRESH_DRAM(){
	
}
//bind the hardware model
void PIN_BIND_MODEL(int midx){
	
}
//start timer
void PIN_START_TIMER(int tidx){
	
}
//stop timer 
void PIN_STOP_TIMER(int tidxm, double * args){
	
}

//start timer
void PIN_DISABLE_TIMERS(){
	
}
void PIN_ENABLE_TIMERS(){
	
}
//start task error injection, start task recording
void PIN_START_TASK(int tidx){
	
}
//end task recording, end task error injection.
void PIN_STOP_TASK(int tidx, double * args){
	
}
void PIN_START_INJECT_ERRORS(){
	
}

void PIN_STOP_INJECT_ERRORS(){
	
}

void pin_start_inject_errors(){
	PIN_START_INJECT_ERRORS();
}
void pin_stop_inject_errors(){
	PIN_STOP_INJECT_ERRORS();
}

void pin_bind_model(int midx){
	PIN_BIND_MODEL(midx);;
}
void pin_refresh_dram(){
	PIN_REFRESH_DRAM();
}

void PIN_UREL_MALLOC(uint64_t addr, uint32_t size){
	
}

double * pin_load_err(pin_error_info * m, double * v){
	m->err = *v; v++;
	m->instr = *v; v++;
	m->total = *v; v++;
	return v;
}
double * pin_store_err(pin_error_info * m, double * v){
	*v = m->err; v++;
	*v = m->instr; v++;
	*v = m->total; v++;
	return v;
}
double * pin_store_task(pin_task_info_t * m, double * v){
	v = pin_store_err(&m->mread, v);
	v = pin_store_err(&m->mwrite, v);
	v = pin_store_err(&m->mem, v);
	*v = m->total; v++;
	return v;
}
double * pin_load_task(pin_task_info_t * m, double * v){
	v = pin_load_err(&m->mread, v);
	v = pin_load_err(&m->mwrite, v);
	v = pin_load_err(&m->mem, v);
	m->total = *v; v++;
	return v;
}


void pin_start_task(int id){
	PIN_START_TASK(id);
}
void pin_stop_task(int id,pin_task_info_t * t){
	double tinfo[10]; 
	double * i;
	pin_task_info_t tsk;
	
	PIN_STOP_TASK(id, tinfo); 
	i=tinfo;
	pin_load_task(&tsk, tinfo);
	*t = tsk;
	
}
void pin_print_err(FILE * out, const char * name, pin_error_info * t){
	fprintf(out, "%s\t%ld\t%ld\t%ld", name, t->err, t->instr, t->total);
}
void pin_print_task(FILE * out, const char * name, pin_task_info_t * t){
	fprintf(out, "%s\t",name);
	pin_print_err(out, "MEM_READ", &t->mread); fprintf(out, "\t");
	pin_print_err(out, "MEM_WRITE", &t->mwrite); fprintf(out, "\t");
	pin_print_err(out, "MEM", &t->mem); fprintf(out, "\t");
	fprintf(out, "%ld", t->total);
}
void pin_disable_timers(){
	PIN_DISABLE_TIMERS();
}
void pin_enable_timers(){
	PIN_ENABLE_TIMERS();
}
void pin_start_timer(int i){
	PIN_START_TIMER(i);
}
void pin_stop_timer(int i, pin_timer_info_t * t){
	PIN_STOP_TIMER(i, &t->inst);
}
void pin_print_timer(FILE * out, const char * name, pin_timer_info_t *t){
	fprintf(out, "%s\t%ld", name, t->inst);
}

void pin_mark_urel(void * ptr, uint32_t size){
	PIN_UREL_MALLOC((uint64_t) ptr, size);
}

