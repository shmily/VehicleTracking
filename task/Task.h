/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-10-18 09:29
#      Filename : Task.h
#   Description : not thing...
#
=============================================================================*/

#ifndef		_TASK_H_
#define		_TASK_H_

#define		_MAX_Task_			8

#define		_Task_Heartbeat_	0
#define		_Task_Btransmit_	1
#define		_Task_Dtransmit_	2
#define		_Task_GPS_Display_	3
#define		_Task_RE_Login_		4

typedef	struct _Task_Period_
{
	int		Counter;
	int		Period;
	int		TimeOut;
	int		Active;
}Task_Period;


void Task_Init(void);
int  isTimeOut(int task_id);
void SetTimeOut(int task_id, int period);
void ClearTimeOut(int task_id);
void DisableTimeOut(int task_id);

#endif
