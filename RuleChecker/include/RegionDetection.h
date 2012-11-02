/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-10-25 19:11
#      Filename : RegionDetection.h
#   Description : not thing...
#
=============================================================================*/

#ifndef __REGION_DETECTION_H__
#define __REGION_DETECTION_H__

#include <time.h>

#include "GPS.h"
#include "myList.h"
#include "ReportConditionList.h"
#include "ReportCondition.h"

// position updata list
typedef struct _RDR_Update_
{
	int 					active;
	int 					Time_active;
	int     				SendCount;
	int 					time_set_index;
	time_t					lastTime;
	InZone_Report_Struct 	*pCondition;
}RDR_Update_Info;


void ZoneInfo_Report_Init(void);
void ZoneInfo_Condition_Check(struct list_head *pHead_InZone, struct list_head *pHead_OutZone, const GpsInfo *pGPS);
int  ZoneInfo_Update(const GpsInfo *pGPS);


#endif
