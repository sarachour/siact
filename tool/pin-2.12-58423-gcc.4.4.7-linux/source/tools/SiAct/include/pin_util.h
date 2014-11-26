#ifndef PIN_UTIL_H
#define PIN_UTIL_H
#include "stdio.h"

typedef enum timer_label {
	TOPAZ_TIMER=0, //topaz routines
	MAIN_TIMER=1, // main computation, no topaz overhead.
	COMM_TIMER=2, //commuication overhead
	COMM_DATA_TIMER=3, //comm data timer
	TASK_TIMER=4,
	TOPAZ_TIMER_SERIALIZE=5, //abstraction
	TOPAZ_TIMER_ABS=6, //abstraction
	TOPAZ_TIMER_DET=7, // outlier detector 
	END
} timer_label;

typedef struct PIN_ERROR_INFO {
	double err;
	double instr;
	double total;
} pin_error_info;

typedef struct PIN_TASK_INFO {
	pin_error_info mread;
	pin_error_info mwrite;
	pin_error_info mem;
	double total;
} pin_task_info_t;

typedef struct PIN_TIMER_INFO{
	double inst;
}pin_timer_info_t;


//bind the hardware model
void PIN_BIND_MODEL(int midx);
void PIN_START_TIMER(int tidx);
void PIN_STOP_TIMER(int tidxm, double * args);
void PIN_START_TASK(int tidx);
void PIN_STOP_TASK(int tidx, double * args);
void PIN_START_INJECT_ERRORS();
void PIN_STOP_INJECT_ERRORS();
void PIN_REFRESH_DRAM();

void pin_start_inject_errors();
void pin_stop_inject_errors();
void pin_bind_model(int midx);
double * pin_store_err(pin_error_info * m,double * v);
double * pin_load_err(pin_error_info * m, double * v);
double * pin_store_task(pin_task_info_t * m, double  * v);
double * pin_load_task(pin_task_info_t * m, double * v);

void pin_refresh_dram();
void pin_start_task(int id);
void pin_stop_task(int id,pin_task_info_t * t);
void pin_print_err(FILE * out, const char * name, pin_error_info * t);
void pin_print_task(FILE * out, const char * name, pin_task_info_t * t);
void pin_start_timer(int i);
void pin_stop_timer(int i, pin_timer_info_t * t);
void pin_print_timer(FILE * out, const char * name, pin_timer_info_t *t);

#endif

