#include "scheduling_simulator.h"
#include "utils.h"

#include <string.h>
#include <time.h>

void hw_suspend(int msec_10){
	int runtask = runningtask;
	task[runtask].status = TASK_WAITING;
	if (task[runtask].priority == 'H'){
		schedule_readyqueue('H');
	} else if (task[runtask].priority == 'L'){
		schedule_readyqueue('L');
	}
	task[runtask].waitingtime = msec_10;
	swapcontext(&task[runtask].context, &go_back_run);
	printf("esgesfhhdehseah\n");
	return;
}

void hw_wakeup_pid(int pid){
	int i=0;
	for (i=0; i<number_of_tasks; i++){
		if (task[i].pid == pid){
			task[i].status = TASK_READY;
			if (task[i].priority == 'H'){
				Hreadyqueue[Htail_readyqueue] = task[i].pid;
				Htail_readyqueue++;
			} else if (task[i].priority == 'L'){
				Lreadyqueue[Ltail_readyqueue] = task[i].pid;
				Ltail_readyqueue++;
			}
		}
	}
	return;
}

int hw_wakeup_taskname(char *task_name){
	int how_many_tasks_are_waken_up = 0;
	int i=0;

	for (i=0; i<number_of_tasks; i++){
		if (!strcmp(task[i].name, task_name)){
			task[i].status = TASK_READY;
			if (task[i].priority == 'H'){
				Hreadyqueue[Htail_readyqueue] = task[i].pid;
				Htail_readyqueue++;
			} else if (task[i].priority == 'L'){
				Lreadyqueue[Ltail_readyqueue] = task[i].pid;
				Ltail_readyqueue++;
			}
			how_many_tasks_are_waken_up++;
		}
	}
	return how_many_tasks_are_waken_up;
}

int hw_task_create(char *task_name)
{
	if (!strcmp(task_name, "Task1") || !strcmp(task_name, "Task2") || !strcmp(task_name, "Task3") || !strcmp(task_name, "Task4") || !strcmp(task_name, "Task5") || !strcmp(task_name, "Task6")) {
		task[number_of_tasks].pid = number_of_tasks;
		strcpy(task[number_of_tasks].name, task_name);
		task[number_of_tasks].status = TASK_READY;
		task[number_of_tasks].time_quantum = 'S';
		task[number_of_tasks].priority = 'L';
		task[number_of_tasks].queueing_time = 0;
		task[number_of_tasks].waitingtime = 0;
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

		if (task[number_of_tasks].priority == 'H') {
			Hreadyqueue[Htail_readyqueue] = task[number_of_tasks].pid;
			Htail_readyqueue++;
		} else if (task[number_of_tasks].priority == 'L') {
			Lreadyqueue[Ltail_readyqueue] = task[number_of_tasks].pid;
			Ltail_readyqueue++;
		}
		return number_of_tasks++; // regard as pid
	} else {
		return -1;
	}
}

int main(){

	getcontext(&TOP);
	//printf("\n--------------------In Shell Mode--------------------\n");
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
			if (!(!strcmp(pch, "Task1") || !strcmp(pch, "Task2") || !strcmp(pch, "Task3") || !strcmp(pch, "Task4") || !strcmp(pch, "Task5") || !strcmp(pch, "Task6"))){
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
			task[number_of_tasks].waitingtime = 0;
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
	//printf("\n--------------------In Simulation Maode--------------------\n");

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
