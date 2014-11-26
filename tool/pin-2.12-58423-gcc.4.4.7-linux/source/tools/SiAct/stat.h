#ifndef STAT_H
#define STAT_H

#include <iostream>
#include <fstream>

#include "stat.h"
#include "string.h"


typedef enum INST_TYPE_T{
	NORMAL, FP, MEM, INST_TYPE_END
} inst_type_t;

#define MAX_NUM_TASKS 10
class Stat {
	
	//per task statistics
	typedef struct STAT_TASK{
		unsigned long int ninst[INST_TYPE_END];
		unsigned int n_memerr;
		unsigned long int n_fperr;
		bool outlier_accept;
		bool true_error;
		bool reexecute;
		unsigned long int ninst_reexecute[INST_TYPE_END];
	} stat_task_t;
	
	typedef struct STAT_TASKSET{
		char name[256];
		//different per-task-statistics
		unsigned int true_positive; //true error & outlier accept
		unsigned int false_negative; //true error & ! outlier accept
		unsigned int false_positive_silent;
		unsigned int true_negative_silent;
		unsigned int false_positive_nerr;
		unsigned int true_negative_nerr;
		unsigned int n_task_reexecute;
		unsigned int n_task_total;
		//emulator statistics
		unsigned long int n_inst[INST_TYPE_END];
		unsigned long int n_inst_reexecute[INST_TYPE_END];
		unsigned int n_memerr;
		unsigned long int n_fperr;
	} stat_taskset_t;
	
		stat_task_t curr_task;
		stat_taskset_t * curr_taskset;
		stat_taskset_t tasksets[MAX_NUM_TASKS];
		int n_tasksets;
	public:
		Stat();
		int curr_taskset_idx;
		int get_taskset(const char * name);
		int add_taskset(const char * name);
		char pack_task_info();
		void mem_err();
		void fp_err();
		void inst(inst_type_t type);
		void inst_reexec(inst_type_t type);
		void outlier_detector(bool accept);
		void output_error(bool iserror);
		void re_execute(bool isreexec);
		void new_task(int idx);
		void consolidate_task();
		void print(ofstream& of);
};

Stat::Stat(){
	this->curr_taskset = NULL;
	this->n_tasksets = 0;
	for(int i=0; i < MAX_NUM_TASKS; i++){
		this->tasksets[i].name[0] = '\0';
		this->tasksets[i].true_positive = 0;
		this->tasksets[i].false_negative = 0;
		this->tasksets[i].false_positive_silent = 0;
		this->tasksets[i].true_negative_silent = 0;
		this->tasksets[i].false_positive_nerr = 0;
		this->tasksets[i].true_negative_nerr = 0;
		this->tasksets[i].n_task_reexecute = 0;
		this->tasksets[i].n_task_total = 0;
		for(int j=0; j < INST_TYPE_END; j++){
			this->tasksets[i].n_inst[j] = 0;
			this->tasksets[i].n_inst_reexecute[j] = 0;
		}
		this->tasksets[i].n_fperr = 0;
		this->tasksets[i].n_memerr = 0;
	 
	}
}

char Stat::pack_task_info(){
	char fperr = 0b1;
	char sramerr = 0b10;
	stat_task_t * t = &(this->curr_task);
	char mask = 0;
	mask |= (t->n_fperr > 0 ? fperr : 0);
	mask |= (t->n_memerr > 0 ? sramerr : 0);
	mask |= (this->curr_taskset_idx << 2);
	return mask;
}
int Stat::get_taskset( const char * name){
	int i = 0;
	for(i=0; i < this->n_tasksets; i++){
		if(strcmp(this->tasksets[i].name, name) == 0){
			return i;
		}
	}
	return 0;
}
int Stat::add_taskset( const char * name){
	int i = this->n_tasksets;
	strcpy(this->tasksets[i].name, name);
	this->n_tasksets++;
	return i;
}

void Stat::mem_err(){
	this->curr_task.n_memerr++;
}

void Stat::fp_err(){
	this->curr_task.n_fperr++;
}

void Stat::inst(inst_type_t type){
	this->curr_task.ninst[type]++;
}
void Stat::inst_reexec(inst_type_t type){
	this->curr_task.ninst_reexecute[type]++;
}
void Stat::outlier_detector(bool accept){
	this->curr_task.outlier_accept = accept;
}

void Stat::output_error(bool iserror){
	this->curr_task.true_error = iserror;
}

void Stat::re_execute(bool isreexec){
	this->curr_task.reexecute = isreexec;
}

void Stat::new_task(int idx){
	this->curr_taskset_idx = idx;
	this->curr_taskset = &this->tasksets[idx];
	this->curr_task.n_memerr = 0;
	this->curr_task.n_fperr = 0;
	this->curr_task.outlier_accept = true;
	this->curr_task.true_error = false;
	this->curr_task.reexecute = false;
	for(int i=0; i < INST_TYPE_END; i++){
		this->curr_task.ninst_reexecute[i] = 0;
		this->curr_task.ninst[i] = 0;
	
	}
	
	
}

void Stat::consolidate_task(){
	stat_task_t * t = &this->curr_task;
	this->curr_taskset->n_memerr += t->n_memerr;
	this->curr_taskset->n_fperr += t->n_fperr;
	
	bool hasinjerr = t->n_memerr > 0 || t->n_fperr > 0;
	this->curr_taskset->true_positive += !t->outlier_accept && t->true_error && hasinjerr ? 1 : 0;
	this->curr_taskset->false_negative += t->outlier_accept && t->true_error && hasinjerr ? 1 : 0;
	this->curr_taskset->false_positive_nerr += !t->outlier_accept && !t->true_error && !hasinjerr ? 1 : 0;
	this->curr_taskset->true_negative_nerr += t->outlier_accept && !t->true_error && !hasinjerr ? 1 : 0;
	this->curr_taskset->false_positive_silent += !t->outlier_accept && !t->true_error && hasinjerr ? 1 : 0;
	this->curr_taskset->true_negative_silent += t->outlier_accept && !t->true_error && hasinjerr ? 1 : 0;
	this->curr_taskset->n_task_total += 1;
	this->curr_taskset->n_task_reexecute += t->reexecute ? 1 : 0;
	for(int i=0; i < INST_TYPE_END; i++){
		this->curr_taskset->n_inst_reexecute[i] += t->ninst_reexecute[i];
		this->curr_taskset->n_inst[i] += t->ninst[i];
	}
	//printf("task: ntask=%d\n", this->curr_taskset->n_task_total);
}

void Stat::print(ofstream& of){
	
	of << "### Instruction Info" << endl;
	of << "NAME" << 
		"\t"<< "# MEM ERR" <<
		"\t" << "# FP ERR" << 
		"\t" << "# MEM INST" << 
		"\t" << "# FP INST" << 
		"\t" << "# NORMAL INST" << 
		endl;
	for(int i=0; i < this->n_tasksets; i++){
		stat_taskset_t * t = &this->tasksets[i];
		of << t->name <<
			"\t"<<t->n_memerr 
		   <<"\t"<<t->n_fperr
		   <<"\t"<<t->n_inst[MEM]
		   <<"\t"<<t->n_inst[FP]
		   <<"\t"<<t->n_inst[NORMAL]
		   <<endl;
	}
	of << endl << endl;
	of << "### Pct Instruction Info" << endl;
	of << "NAME" << 
		"\t"<< "PCT MEM ERR" <<
		"\t" << "PCT FP ERR" << 
		"\t" << "PCT MEM INST" << 
		"\t" << "PCT FP INST" << 
		"\t" << "PCT NORMAL INST" << 
		endl;
	for(int i=0; i < this->n_tasksets; i++){
		stat_taskset_t * t = &this->tasksets[i];
		long int total = t->n_inst[MEM] + t->n_inst[FP] + t->n_inst[NORMAL];
		of << t->name <<
			"\t"<<(100*(float)t->n_memerr)/(t->n_inst[MEM])
		   <<"\t"<<(100*(float)t->n_fperr)/(t->n_inst[FP])
		   <<"\t"<<(100*(float)t->n_inst[MEM])/(total)
		   <<"\t"<<(100*(float)t->n_inst[FP])/total
		   <<"\t"<<(100*(float)t->n_inst[NORMAL])/total
		   <<endl;
	}
	of << endl << endl;
	
	of << "### Task Info" << endl;
	of << "NAME" << 
		"\t"<< "# TASKS" <<
		"\t"<< "# TRUE POSITIVE" <<
		"\t" << "# FALSE NEGATIVE" << 
		"\t" << "# FALSE POSITIVE [silent]" << 
		"\t" << "# TRUE NEGATIVE [silent]" << 
		"\t" << "# FALSE POSITIVE [no error]" << 
		"\t" << "# TRUE NEGATIVE [no error]" << 
		"\t" << "# RE-EXECUTIONS" << 
		endl;
	for(int i=0; i < this->n_tasksets; i++){
		stat_taskset_t * t = &this->tasksets[i];
		//printf("N TASKS[%d: %d,%d] %d\n", PIN_GetPid(), getHWModelIDX(), i, t->n_task_total);
		of << t->name
		   <<"\t"<<t->n_task_total 
		   <<"\t"<<t->true_positive 
		   <<"\t"<<t->false_negative 
		   <<"\t"<<t->false_positive_silent 
		   <<"\t"<<t->true_negative_silent 
		   <<"\t"<<t->false_positive_nerr 
		   <<"\t"<<t->true_negative_nerr
		   <<"\t"<<t->n_task_reexecute 
		   <<endl;
		
	}
}


#endif
