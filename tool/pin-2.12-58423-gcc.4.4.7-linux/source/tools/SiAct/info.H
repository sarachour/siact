#ifndef INFO_H
#define INFO_H
#include <set>

extern uint32_t N_SYS_CORRUPTIONS;

typedef struct TIMER_INFO_T {
	double N_INSTR;
} timer_info_t;

typedef struct ERROR_INFO_T {
	double N_INSTR;
	double N_TOTAL;
	double N_ERR;
} error_info_t;

typedef struct TASK_INFO_T {
	error_info_t MEM_READ;
	error_info_t MEM_WRITE;
	error_info_t DRAM;
	error_info_t MEM;
	error_info_t FP;
	timer_info_t TIMER;
} task_info_t;

typedef enum TASK_ENUM_T{
	MEM_WRITE,
	MEM_READ,
	MEM,
	FP,
	DRAM,
	TOTAL
} task_enum_t;


#define N_TASKS 10
#define N_TIMERS 10

task_info_t ctask[N_TASKS];
timer_info_t ctimer[N_TIMERS];
std::set<int> active_timers;
std::set<int> active_tasks;

task_info_t tasks[N_TASKS];
timer_info_t timers[N_TIMERS];

void init_err_info(error_info_t * err){
	err->N_INSTR = 0;
	err->N_TOTAL = 0;
	err->N_ERR = 0;
}
double * store_err_info(error_info_t * e, double * d){
	d[0] = e->N_ERR;
	d[1] = e->N_INSTR;
	d[2] = e->N_TOTAL;
	return &d[3];
}
void sum_err_info(error_info_t * e, error_info_t * t){
	e->N_ERR += t->N_ERR;
	e->N_TOTAL += t->N_TOTAL;
	e->N_INSTR += t->N_INSTR;
}

void init_timer(timer_info_t * tim){
	tim->N_INSTR = 0;
}
double * store_timer(timer_info_t * e, double * d){
	d[0] = e->N_INSTR;
	return &d[1];
}
void sum_timer(timer_info_t * e, timer_info_t * t){
	e->N_INSTR += t->N_INSTR;
}

typedef struct LOG_TIMER_INFO {
	int instr;
	int id;
} log_timer_info_t;

#define N_ENTRIES 300
char filename[256];
int ENTRY_IDX = 0;
log_timer_info_t TIMER_BUFFER[N_ENTRIES];

void log_timer(int id, timer_info_t * e){
	if(ENTRY_IDX == N_ENTRIES - 1){
		FILE * fp = fopen(filename,"w+");
		for(int i=0; i < N_ENTRIES; i++){
			log_timer_info_t tim = TIMER_BUFFER[i];
			fprintf(fp, "%d\t%d\n", tim.id,tim.instr);
		}
		fclose(fp);
		ENTRY_IDX = 0;
		
	}
	TIMER_BUFFER[ENTRY_IDX].instr = (int) e->N_INSTR;
	TIMER_BUFFER[ENTRY_IDX].id = id;
	ENTRY_IDX++;
	
}
void timer_set_filename(int idx){
	sprintf(filename,"emul.timer.%d.out", idx);
}

void init_task_info( task_info_t * task){
	init_err_info(&task->MEM_READ);
	init_err_info(&task->MEM_WRITE);
	init_err_info(&task->MEM);
	init_err_info(&task->DRAM);
	init_err_info(&task->FP);
	init_timer(&task->TIMER);
}

double * store_task_info(task_info_t * e, double * d){
	double * ptr = d;
	ptr = store_err_info(&e->MEM_READ, ptr);
	ptr = store_err_info(&e->MEM_WRITE, ptr);
	ptr = store_err_info(&e->MEM, ptr);
	ptr = store_err_info(&e->DRAM, ptr);
	ptr = store_err_info(&e->FP, ptr);
	ptr = store_timer(&e->TIMER, ptr);
	return ptr;
}

void sum_task_info(task_info_t * e, task_info_t * t){
	sum_err_info(&e->MEM_READ, &t->MEM_READ);
	sum_err_info(&e->MEM_WRITE, &t->MEM_WRITE);
	sum_err_info(&e->MEM, &t->MEM);
	sum_err_info(&e->DRAM, &t->DRAM);
	sum_err_info(&e->FP, &t->FP);
	sum_timer(&e->TIMER, &e->TIMER);
}

void init_all(){
	for(int i=0; i < N_TASKS; i++){
		init_task_info(&tasks[i]);
		init_task_info(&ctask[i]);
	}
	for(int i=0; i < N_TIMERS; i++){
		init_timer(&timers[i]);
		init_timer(&ctimer[i]);
	}	
	
}

void timer_start(int id){
	init_timer(&ctimer[id]);
	//printf("PIN TIMER START:%f\n", ctimer[id].N_INSTR);
	active_timers.insert(id);
}
void timer_stop(int id, double * args){
	sum_timer(&timers[id], &ctimer[id]);
	//printf("PIN TIMER STOP:%f\n", ctimer[id].N_INSTR);
	store_timer(&ctimer[id], args);
	active_timers.erase(id);
}
void task_start(int id){
	init_task_info(&ctask[id]);
	active_tasks.insert(id);
}
void task_stop(int id, double * args){
	store_task_info(&ctask[id], args);
	sum_task_info(&tasks[id], &ctask[id]);
	active_tasks.erase(id);
}
int task_active_count(){
	return active_tasks.size();
}
void timer_inst(){
	for(std::set<int>::iterator it =active_timers.begin(); it != active_timers.end(); ++it){
		ctimer[*it].N_INSTR++;
	}
}

void timer_inst_m(int n){
	for(std::set<int>::iterator it =active_timers.begin(); it != active_timers.end(); ++it){
		ctimer[*it].N_INSTR+= n;
	}
}
inline void _task_inst(task_info_t * t, task_enum_t type, bool instr, bool inj){
	error_info_t * inf;
	switch(type){
		case MEM_READ: 
			inf = &t->MEM_READ; 
			break;
		case MEM_WRITE: 
			inf = &t->MEM_WRITE;  //read count
			break;
		case MEM: 
			inf = &t->MEM; 
			t->TIMER.N_INSTR++; //instruction count
			break;
		case FP:
			inf = &t->FP;
			t->TIMER.N_INSTR++;
			break;
		case DRAM:
			inf = &t->DRAM;
			break;
		case TOTAL:
			t->TIMER.N_INSTR++; //instruction count
			return;
			break;
		default:
			return;
	}
	inf->N_TOTAL++;
	if(instr) inf->N_INSTR++;
	if(inj) inf->N_ERR++;
}

inline void task_inst(task_enum_t type, bool instr, bool inj){
	for(std::set<int>::iterator it =active_tasks.begin(); it != active_tasks.end(); ++it){
		_task_inst(&ctask[*it], type, instr, inj);
	}
}
void print_timer_hdr(FILE * out){
	fprintf(out, "name\ttotal");
}
void print_timer(FILE * out, const char * name, timer_info_t * t){
	fprintf(out, "%s\t%f", name, t->N_INSTR);
}
void print_err_hdr(FILE * out){
	fprintf(out, "name\terrors\tinstrumented\ttotal");
}
void print_err(FILE * out, const char * err, error_info_t * t){
	fprintf(out, "%s\t%f\t%f\t%f", err, t->N_ERR, t->N_INSTR, t->N_TOTAL);
}
void print_task_hdr(FILE * out){
	fprintf(out, "task-name\t");
	print_err_hdr(out); fprintf(out,"\t");
	print_err_hdr(out); fprintf(out,"\t");
	print_err_hdr(out); fprintf(out,"\t");
	fprintf(out, "name\terrors\t");
	print_timer_hdr(out);
}
void print_task(FILE* out, const char * name, task_info_t * t){
	fprintf(out, "%s\t", name);
	print_err(out, "MEM_READ", 	&t->MEM_READ); fprintf(out, "\t");
	print_err(out, "MEM_WRITE", 	&t->MEM_WRITE); fprintf(out, "\t");
	print_err(out, "MEM", 	&t->MEM); fprintf(out, "\t");
	print_err(out, "DRAM", 	&t->DRAM); fprintf(out, "\t");
	print_err(out, "FP", 	&t->FP); fprintf(out, "\t");
	print_timer(out, "TOTAL", &t->TIMER);
}
void print_all(FILE * out){
	char name[255];
	fprintf(out,"### TASK INFO ###\n");
	print_task_hdr(out); fprintf(out,"\n");
	for(int i=0; i < N_TASKS; i++){
		sprintf((char *)name, "TASK[%d]", i);
		print_task(out, name, &tasks[i]);
		fprintf(out,"\n");
	}
	fprintf(out,"\n\n");
	fprintf(out,"### TIMER INFO ###\n");
	print_timer_hdr(out); fprintf(out,"\n");
	for(int i=0; i < N_TIMERS; i++){
		sprintf((char *)name, "TIMER[%d]", i);
		print_timer(out, name, &timers[i]);
		fprintf(out,"\n");
	}
}
#endif
