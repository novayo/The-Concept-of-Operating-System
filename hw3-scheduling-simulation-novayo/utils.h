#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <ucontext.h>
#include <sys/time.h>
#define TIME 10
#define BUFFER 1000000
#define buffer 100
#define True 1
#define False 0
#define MAX_QUEUE 10000
#define MAX_TASKS 20

typedef struct struct_pcb{
	ucontext_t context;
	int pid;
	char name[buffer]; //like task1
	int status;
	char time_quantum;
	char priority;
	int queueing_time;
	int waitingtime;
} pcb;

ucontext_t TOP, go_back_run, tmp;
int number_of_tasks;
pcb task[MAX_TASKS];
int Hreadyqueue[MAX_QUEUE];
int Hhead_readyqueue;
int Htail_readyqueue;
int Lreadyqueue[MAX_QUEUE];
int Lhead_readyqueue;
int Ltail_readyqueue;
int times_for_high_priority_rr_20ms;
int times_for_low_priority_rr_20ms;
int times_for_high_priority_rr_10ms;
int times_for_low_priority_rr_10ms;

int first_run = True;
int is_empty_readyqueue(char priority);
void schedule_readyqueue(char priority);
void print_readyqueue();
void uninit_time();
void timerstart(int i);
void ctrl_z();
void do_nothing();

int runningtask;
int run_or_not;
int inttimerstart;
int __100_10msec = 0;
void run(){
	getcontext(&go_back_run);
	int i=0;
	if (is_empty_readyqueue('H') == False){
		for (i=0; i<number_of_tasks; i++){
			if (task[i].pid == Hreadyqueue[Hhead_readyqueue]){
				task[i].status = TASK_RUNNING;
				runningtask = i;
				signal(SIGTSTP, ctrl_z); 
				if (times_for_high_priority_rr_20ms == 1) times_for_high_priority_rr_20ms = 1;
				else if (task[i].priority == 'L') times_for_high_priority_rr_20ms = 1;
				break;
			}
		}
	} else if (is_empty_readyqueue('L') == False){
		for (i=0; i<number_of_tasks; i++){
			if (task[i].pid == Lreadyqueue[Lhead_readyqueue]){
				task[i].status = TASK_RUNNING;
				runningtask = i;
				signal(SIGTSTP, ctrl_z); 
				if (times_for_low_priority_rr_20ms == 1) times_for_low_priority_rr_20ms = 1;
				else if (task[i].priority == 'L') times_for_low_priority_rr_20ms = 1;
				break;
			}
		}
	}

	inttimerstart = False;
	timerstart(TIME); //10ms
	while (inttimerstart == False);
	swapcontext(&go_back_run, &task[i].context); //go to func
	if (is_empty_readyqueue('H') == False){
		for (i=0; i<number_of_tasks; i++){
			if (task[i].pid == Hreadyqueue[Hhead_readyqueue]){
				task[i].status = TASK_RUNNING;
				runningtask = i;
				signal(SIGTSTP, ctrl_z); 
				if (task[i].priority == 'L') times_for_high_priority_rr_20ms = 1;
				break;
			}
		}
		swapcontext(&tmp, &task[i].context); //go to func
		task[i].status = TASK_TERMINATED;
		schedule_readyqueue('H');
	} else if (is_empty_readyqueue('L') == False){
		for (i=0; i<number_of_tasks; i++){
			if (task[i].pid == Lreadyqueue[Lhead_readyqueue]){
				task[i].status = TASK_RUNNING;
				runningtask = i;
				signal(SIGTSTP, ctrl_z); 
				if (task[i].priority == 'L') times_for_low_priority_rr_20ms = 1;
				break;
			}
		}
		swapcontext(&tmp, &task[i].context); //go to func
		task[i].status = TASK_TERMINATED;
		schedule_readyqueue('L');
	}
	while(1);
}

void ps();
void round_robin(){
	if (inttimerstart == True){
		signal(SIGTSTP, do_nothing);

		int i=0;
		for (i=0; i<number_of_tasks; i++){
			if (task[i].status == TASK_READY){
				task[i].queueing_time++;
			}
		}

		// waiting time
		int k = 0;
		for(k=0; k<number_of_tasks; k++){
			if (task[k].status == TASK_WAITING){
				if (task[k].waitingtime == 0){
					// When finish
					task[k].status = TASK_READY;
					if (task[k].priority == 'H'){
						Hreadyqueue[Htail_readyqueue] = task[k].pid;
						Htail_readyqueue++;
					} else if (task[k].priority == 'L'){
						Lreadyqueue[Ltail_readyqueue] = task[k].pid;
						Ltail_readyqueue++;
					}
				} else{
					task[k].waitingtime--;
				}
			}
		}

		if (is_empty_readyqueue('H') == False){
			int i=0;
			for (i=0; i<number_of_tasks; i++){
				if (task[i].pid == Hreadyqueue[Hhead_readyqueue]){
					break;
				}
			}
			if (task[i].time_quantum == 'L'){
				if (times_for_high_priority_rr_20ms == 0){
					task[i].status = TASK_READY;
					schedule_readyqueue('H');
					swapcontext(&task[i].context, &go_back_run);
				} else if (times_for_high_priority_rr_20ms == 1){
					times_for_high_priority_rr_20ms = 0;
				}
			} else if (task[i].time_quantum == 'S'){
				task[i].status = TASK_READY;
				schedule_readyqueue('H');
				swapcontext(&task[i].context, &go_back_run);
			}        
		} else if (is_empty_readyqueue('L') == False){
			int i=0;
			for (i=0; i<number_of_tasks; i++){
				if (task[i].pid == Lreadyqueue[Lhead_readyqueue]){
					break;
				}
			}
			if (task[i].time_quantum == 'L'){
				if (times_for_low_priority_rr_20ms == 0){
					task[i].status = TASK_READY;
					schedule_readyqueue('L');
					swapcontext(&task[i].context, &go_back_run);
				} else if (times_for_low_priority_rr_20ms == 1){
					times_for_low_priority_rr_20ms = 0;
				}
			} else if (task[i].time_quantum == 'S'){
				task[i].status = TASK_READY;
				schedule_readyqueue('L');
				swapcontext(&task[i].context, &go_back_run);
			}        
		}
	} else{
		inttimerstart = True;
	}
}

void init_readyqueue(){
	// run in simulation -> got how many task in number_of_tasks
	int i=0;
	for (i=0; i<number_of_tasks; i++){
		if (task[i].priority == 'H'){
			Hreadyqueue[i] = task[i].pid;
			Htail_readyqueue++;
		} else if (task[i].priority == 'L'){
			Lreadyqueue[i] = task[i].pid;
			Ltail_readyqueue++;
		}
	}
}

int pre_number_of_tasks;
void enreadyqueue(){
	int i = 0;
	for (i=0; i<(number_of_tasks - pre_number_of_tasks); i++){
		if (task[i].priority == 'H'){
			Hreadyqueue[Htail_readyqueue] = task[pre_number_of_tasks + i].pid;
			Htail_readyqueue++;
		} else if (task[i].priority == 'L'){
			Lreadyqueue[Ltail_readyqueue] = task[pre_number_of_tasks + i].pid;
			Ltail_readyqueue++;
		}
	}
}

void schedule_readyqueue(char priority){
	if (priority == 'H'){
		if (Hreadyqueue[Hhead_readyqueue] == -1) {
			Hhead_readyqueue++;
			schedule_readyqueue('H');
			return;
		}
		if (Hhead_readyqueue == Htail_readyqueue){
			printf("Hreadyqueue is empty...\n");
			return;
		}
		int j=0;
		for (j=0; j<number_of_tasks; j++){
			if (task[j].pid == Hreadyqueue[Hhead_readyqueue]){
				if (task[j].status == TASK_TERMINATED || task[j].status == TASK_WAITING){
					Hhead_readyqueue++;
					return;
				}
			}
		}
		Hreadyqueue[Htail_readyqueue] = Hreadyqueue[Hhead_readyqueue];
		Htail_readyqueue++;
		Hhead_readyqueue++;
	} else if (priority == 'L'){
		if (Lreadyqueue[Lhead_readyqueue] == -1) {
			Lhead_readyqueue++;
			schedule_readyqueue('L');
			return;
		}
		if (Lhead_readyqueue == Ltail_readyqueue){
			printf("Lreadyqueue is empty...\n");
			return;
		}
		int j=0;
		for (j=0; j<number_of_tasks; j++){
			if (task[j].pid == Lreadyqueue[Lhead_readyqueue]){
				if (task[j].status == TASK_TERMINATED || task[j].status == TASK_WAITING){
					Lhead_readyqueue++;
					return;
				}
			}
		}
		Lreadyqueue[Ltail_readyqueue] = Lreadyqueue[Lhead_readyqueue];
		Ltail_readyqueue++;
		Lhead_readyqueue++;
	}
}

int is_empty_readyqueue(char priority){
	if (priority == 'H'){
		if (Hhead_readyqueue == Htail_readyqueue){
			return True;
		} else{
			return False;
		}
	} else if (priority == 'L'){
		if (Lhead_readyqueue == Ltail_readyqueue){
			return True;
		} else{
			return False;
		}
	} else{
		return -1;
	}
}

void print_readyqueue(){
	printf("\nHigh Priority Ready Queue: \n");
	int i = Hhead_readyqueue;
	for (; i<Htail_readyqueue; i++) {
		char name[buffer] = {'\0'};
		int j=0;
		for (j=0; j<number_of_tasks; j++){
			if (task[j].pid == Hreadyqueue[i]){
				strcpy(name, task[j].name);
				break;
			}
		}
		printf("(%d)%s | ", Hreadyqueue[i], name);
	}
	printf("\n");
	printf("\nLigh Priority Ready Queue: \n");
	i = Lhead_readyqueue;
	for (; i<Ltail_readyqueue; i++) {
		char name[buffer] = {'\0'};
		int j=0;
		for (j=0; j<number_of_tasks; j++){
			if (task[j].pid == Lreadyqueue[i]){
				strcpy(name, task[j].name);
				break;
			}
		}
		printf("(%d)%s | ", Lreadyqueue[i], name);
	}
	printf("\n");
}

void uninit_time()  
{  
	struct itimerval value;  
	value.it_value.tv_sec = 0;  
	value.it_value.tv_usec = 0;  
	value.it_interval = value.it_value;  
	setitimer(ITIMER_REAL, &value, NULL);  
}

void timerstart(int i){
	struct itimerval value, ovalue; 
	signal(SIGALRM, round_robin);
	int sec = (i*1000)/1000000;
	int usec = (i*1000)%1000000;
	value.it_value.tv_sec = sec;
	value.it_value.tv_usec = usec;
	value.it_interval.tv_sec = sec;
	value.it_interval.tv_usec = usec;
	setitimer(ITIMER_REAL, &value, &ovalue);
}

void ctrl_z(int signal)
{
	pre_number_of_tasks = number_of_tasks;
	uninit_time();
	first_run = False;
	setcontext(&TOP);
}

void remove_pid(int pid){
	int i=0;
	for (i=0; i<number_of_tasks; i++){
		if (task[i].pid == pid) break;
	}
	if (i == number_of_tasks) {
		printf("No such pid...\n");
	} else{
		if (task[i].status == TASK_READY){
			int j=0;
			if (task[i].priority == 'H'){
				for (j=Hhead_readyqueue; j<Htail_readyqueue; j++){
					if (Hreadyqueue[j] == task[i].pid){
						Hreadyqueue[j] = -1;
						break;
					}
				}
			} else if (task[i].priority == 'L'){
				for (j=Lhead_readyqueue; j<Ltail_readyqueue; j++){
					if (Lreadyqueue[j] == task[i].pid){
						Lreadyqueue[j] = -1;
						break;
					}
				}
			}
		}
		task[i].pid = -1;
		strcpy(task[i].name, "");
		task[i].status = TASK_TERMINATED;
		task[i].time_quantum = '\0';
		task[i].priority = '\0';
		task[i].queueing_time = 0;
	}
	return;
}

void ps(){
	int i = 0;
	printf("number_of_tasks = %d\n", number_of_tasks);
	for (i=0; i < number_of_tasks; i++){
		if (task[i].pid == -1) continue; //removed task
		char status[buffer] = {'\0'};
		if (task[i].status == TASK_RUNNING) strcpy(status, "TASK_RUNNING");
		else if (task[i].status == TASK_READY) strcpy(status, "TASK_READY");
		else if (task[i].status == TASK_WAITING) strcpy(status, "TASK_WAITING");
		else if (task[i].status == TASK_TERMINATED) strcpy(status, "TASK_TERMINATED");
		printf("%d %s %s %d %c %c\n", 
				task[i].pid, 
				task[i].name, 
				status,
				task[i].queueing_time,
				task[i].priority,
				task[i].time_quantum); //1 task1 TASK_READY 50 H L
	}
	return;
}

void do_nothing(){
	// do_nothing
}

#endif
