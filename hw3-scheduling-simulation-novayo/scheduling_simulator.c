#include "scheduling_simulator.h"
#include "utils.h"

#include <string.h>
#include <time.h>

void hw_suspend(int msec_10){
	int taskpid = runningtask;
	enwaitingqueue(taskpid);
	clock_t tmr1, tmr2;
	tmr1 = clock();
	while(True){
		tmr2 = clock();
		long int currenttime = (long int)((double)(tmr2 - tmr1)/CLOCKS_PER_SEC);
		long int limit = msec_10*10000;
		if (currenttime > limit){
			break;
		}
	}
	if (schedule_waitingqueue(taskpid));
	return;
}

void hw_wakeup_pid(int pid){
	schedule_waitingqueue(pid);
	return;
}

int hw_wakeup_taskname(char *task_name){
	int how_many_tasks_are_waken_up = 0;
	int i=0;
	
	for(i=0; i<head_task_waiting; i++){
		if (task_waiting_queue[i] == -1) continue;
		int j=0;
		for (j=0; j<number_of_tasks; j++){
			if (task[j].pid == task_waiting_queue[i] && !strcmp(task[j].name, task_name)){
				if (schedule_waitingqueue(task[j].pid) == True) how_many_tasks_are_waken_up++;
				break;
			}
		}
	}
    return how_many_tasks_are_waken_up;
}

int hw_task_create(char *task_name){
	if (!strcmp(task_name, "Task1") || !strcmp(task_name, "Task2") || !strcmp(task_name, "Task3") || !strcmp(task_name, "Task4") || !strcmp(task_name, "Task5") || !strcmp(task_name, "Task6")){
		return True; // regard as pid
	} else{
		return -1;
	}
}

int main(){

	getcontext(&TOP);
	printf("\n--------------------In Shell Mode--------------------\n");
	signal(SIGTSTP, do_nothing);
	int add_or_not = False;
	char enter[BUFFER] = {'\0'};
	while(True){
		char tmpenter[buffer] = {'\0'};
		scanf("%[^\n]%*c", &tmpenter);
		if (!strcmp(tmpenter, "start")) {
			break;
		}

		char tmptmpenter[buffer] = {'\0'};
		strcpy(tmptmpenter, tmpenter);
		char *pch;
		pch = strtok(tmptmpenter, " ");

		if (!strcmp(pch, "add")) {
			pch = strtok (NULL, " "); // get task name
			int pid = hw_task_create(pch);
			if (pid == -1){
				printf("There is no function named \"task_name\"...\n");
				continue;
			}

			task[number_of_tasks].pid = number_of_tasks;
			strcpy(task[number_of_tasks].name, pch);
			task[number_of_tasks].status = TASK_READY;

			pch = strtok (NULL, " "); // get -t
			if (pch == NULL) {
				task[number_of_tasks].time_quantum = 'S';
				task[number_of_tasks].priority = 'L';
			} else{
				pch = strtok (NULL, " "); // get time_quantum
				task[number_of_tasks].time_quantum = pch[0];
				pch = strtok (NULL, " "); // get -p
				if (pch == NULL) {
					task[number_of_tasks].priority = 'L';
				} else{
					pch = strtok (NULL, " "); // get priority
					task[number_of_tasks].priority = pch[0];
				}
			}
			//指定栈
			getcontext(&task[number_of_tasks].context);
			task[number_of_tasks].context.uc_stack.ss_sp = malloc(BUFFER);
			task[number_of_tasks].context.uc_stack.ss_size = BUFFER;
			task[number_of_tasks].context.uc_link = &tmp;
			if (!strcmp(task[number_of_tasks].name, "Task1")) makecontext(&task[number_of_tasks].context, task1, 0);
			else if (!strcmp(task[number_of_tasks].name, "Task2")) makecontext(&task[number_of_tasks].context, task2, 0);
			else if (!strcmp(task[number_of_tasks].name, "Task3")) makecontext(&task[number_of_tasks].context, task3, 0);
			else if (!strcmp(task[number_of_tasks].name, "Task4")) makecontext(&task[number_of_tasks].context, task4, 0);
			else if (!strcmp(task[number_of_tasks].name, "Task5")) makecontext(&task[number_of_tasks].context, task5, 0);
			else if (!strcmp(task[number_of_tasks].name, "Task6")) makecontext(&task[number_of_tasks].context, task6, 0);
			task[number_of_tasks].queueing_time = 0;
			number_of_tasks++;
			add_or_not = True;
		} else if (!strcmp(pch, "remove")) {
			pch = strtok (NULL, " "); // get pid
			remove_pid(atoi(pch));
		} else if (!strcmp(pch, "ps")) {
			ps();
		} else{
			printf("Wrong command...\n");
			continue;
		}

		strcat(enter, tmpenter);
		strcat(enter, "\n");
	}
	printf("\n--------------------In Simulation Maode--------------------\n");
	signal(SIGTSTP, ctrl_z); 
	printf("simulating...\n");
	if (first_run == True){
		init_readyqueue();
		add_or_not = False;
	} else if(add_or_not == True){
		enreadyqueue();
		add_or_not = False;
	}
	run();
	return 0;
}
