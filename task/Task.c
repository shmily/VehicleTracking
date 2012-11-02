/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-10-18 09:29
#      Filename : Task.c
#   Description : not thing...
#
=============================================================================*/

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/time.h>
#include <errno.h>

#include "Task.h"


static Task_Period Task_Control[_MAX_Task_];


void Tick_Init(void);

void Task_Init(void)
{
	int 	i;
	char 	*pdest;

	for(i=0; i<_MAX_Task_; i++){

		pdest = (char *)&(Task_Control[i]);
		memset(pdest, 0, sizeof(Task_Period));
	}

	Tick_Init();
}

int isTimeOut(int task_id)
{
	if(Task_Control[task_id].Active == 1){
		
		return Task_Control[task_id].TimeOut;
	} else {
		
		return 0;
	}
}

void SetTimeOut(int task_id, int period)
{
	Task_Control[task_id].Active  = 1;
	Task_Control[task_id].TimeOut = 0;
	Task_Control[task_id].Counter = 0;
	Task_Control[task_id].Period  = period;
}

void ClearTimeOut(int task_id)
{
	Task_Control[task_id].Active  = 1;
	Task_Control[task_id].TimeOut = 0;
	Task_Control[task_id].Counter = 0;
}

void DisableTimeOut(int task_id)
{
	Task_Control[task_id].Active  = 0;
	Task_Control[task_id].TimeOut = 0;
	Task_Control[task_id].Counter = 0;
	Task_Control[task_id].Period  = 0;
}


// execute every 10 seconds
void TickHandle( int sig )
{
	int i;

	for(i=0; i<_MAX_Task_; i++){

		if( (Task_Control[i].Active == 1)&&(Task_Control[i].TimeOut == 0) ){

			Task_Control[i].Counter = Task_Control[i].Counter + 1;

			if(Task_Control[i].Counter >= Task_Control[i].Period){

				Task_Control[i].TimeOut = 1;
			}
		}
	}
	fprintf(stderr, "%s : > Tick ...\n", __func__);
	alarm(10);
}


void Tick_Init(void)
{
	signal(SIGALRM,TickHandle);
	alarm(10);						// 10 seconds
}
