/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-10-27 09:20
#      Filename : SpeedDetection.h
#   Description : not thing...
#
=============================================================================*/

#ifndef __SPEED_DETECTION_H__
#define __SPEED_DETECTION_H__

#include <time.h>
#include "GPS.h"

// speed updata list
typedef struct _SpeedUpdate_Info_
{
	int 					active;
	int 					Time_active;
	int     				SendCount;
	int 					time_set_index;
	time_t					lastTime;
	SpeedAnomaly_Report 	*pCondition;
}SpeedUpdate_Info;


void Speed_Report_Init(void);
void Speed_Condition_Check(struct list_head *pHead, const GpsInfo *pGPS);
int SpeedReport_Update(const GpsInfo *pGPS);


#endif
