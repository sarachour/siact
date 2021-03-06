#ifndef PIN_UTIL_H
#define PIN_UTIL_H
#include "stdio.h"
#include <stdint.h>
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
void PIN_DISABLE_TIMERS();
void PIN_ENABLE_TIMERS();
void PIN_START_TIMER(int tidx);
void PIN_START_TIMER(int tidx);
void PIN_STOP_TIMER(int tidxm, double * args);
void PIN_START_TASK(int tidx);
void PIN_STOP_TASK(int tidx, double * args);
void PIN_START_INJECT_ERRORS();
void PIN_STOP_INJECT_ERRORS();
void PIN_REFRESH_DRAM();
void PIN_UREL_MALLOC(uint64_t addr, uint32_t size);

void pin_disable_timers();
void pin_enable_timers();
void pin_start_inject_errors();
void pin_stop_inject_errors();
void pin_bind_model(int midx);
double * pin_store_err(pin_error_info * m,double * v);
double * pin_load_err(pin_error_info * m, double * v);
double * pin_store_task(pin_task_info_t * m, double  * v);
double * pin_load_task(pin_task_info_t * m, double * v);
void pin_mark_urel(void * ptr, uint32_t size);
void pin_refresh_dram();
void pin_start_task(int id);
void pin_stop_task(int id,pin_task_info_t * t);
void pin_print_err(FILE * out, const char * name, pin_error_info * t);
void pin_print_task(FILE * out, const char * name, pin_task_info_t * t);
void pin_start_timer(int i);
void pin_stop_timer(int i, pin_timer_info_t * t);
void pin_print_timer(FILE * out, const char * name, pin_timer_info_t *t);

#define DBLUREL(v) pin_mark_urel(&v, sizeof(double));
#define DBLNUREL(v,n) pin_mark_urel(v, sizeof(double)*n);


#define FPUREL(v) pin_mark_urel(&v, sizeof(float));
#define FPNUREL(v,n) pin_mark_urel(v, sizeof(float)*n);

#define IUREL(v) pin_mark_urel(&v, sizeof(int));
#define DBLUREL(v) pin_mark_urel(&v, sizeof(double));

#define UNKUREL(v,s) pin_mark_urel(&v, sizeof(s));
#define UNKNUREL(v,s,n) pin_mark_urel(v, sizeof(s)*n);
#define UNKNUREL2(v,n) pin_mark_urel(v, n);

#define FPDEF(v) float v; FPUREL(v);
#define IDEF(v) int v; IUREL(v);
#define DBLDEF(v) double v; DBLUREL(v);
#endif

