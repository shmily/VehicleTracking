/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-10-27 09:20
#      Filename : SpeedDetection.c
#   Description : not thing...
#
=============================================================================*/

#include <time.h>
#include <stdint.h>
#include <string.h>

#include "Debug.h"
#include "TimeCompare.h"
#include "Interval_Locate.h"

#include "ZoneInfo.h"
#include "Btype_Control.h"
#include "myList.h"
#include "RuleList.h"
#include "ReportCondition.h"
#include "ReportConditionList.h"
#include "SpeedDetection.h"

static char					Speed_Data_Buff[256];
static SpeedUpdate_Info  	ActiveCondition;

static void Speed_AKV_Enpacket(const int action, const GpsInfo *pGPS, const SpeedAnomaly_Report *pCondition, char *pbuff, int *len);

void Speed_Report_Init(void)
{
	memset((char *)&ActiveCondition, 0, sizeof(SpeedUpdate_Info));
}

void Speed_Condition_Check(struct list_head *pHead, const GpsInfo *pGPS)
{
	struct list_head 	*plist;
	SpeedAnomaly_Report *pCondition;
	Time_D 				*pTime;

	int  				mach = 0;
	int 				TimeSet_sum;
	int 				i;
	TimeSlot_Struct		*pTimeSlot;		

	// first, we find a active time proid

	if(ActiveCondition.Time_active == 1){						// we aleady have a condition, we check it

		pCondition = ActiveCondition.pCondition;
		pTime = (Time_D *)&((pCondition->TimeSet).BE_Time[(ActiveCondition.time_set_index)].Etime);

		if( Is_BeyondTime_D( pTime ) == 1 ){				// out of the time


			DEBUG_MSG("%s : >>>> Delete the rule...\n", __func__);
			ActiveCondition.active = 0;
			ActiveCondition.Time_active = 0;
			ActiveCondition.SendCount = 0;

			ReportCondition_Del(&pCondition->list);
			free(pCondition);

			pCondition = NULL;
			ActiveCondition.pCondition = NULL;
		}

	} else {	// else, we search for a active condition

		list_for_each(plist, pHead){

			pCondition = list_entry(plist, SpeedAnomaly_Report, list);

			TimeSet_sum = pCondition->TimeSet.TimeSet_Count;

			for(i=0; i<TimeSet_sum; i++){
			
				pTimeSlot = &(pCondition->TimeSet.BE_Time[i]);

				if( (Is_BeyondTime_D( &(pTimeSlot->Btime) ) == 1) && 
					(Is_BeyondTime_D( &(pTimeSlot->Etime) ) == 0) ) {
					
					mach = 1;
					break;
				}
			}

			if(mach == 1){
				break;
			}

		}

		if(plist!=pHead){	// we get the rule

			DEBUG_MSG("%s : > we get the rule...\n", __func__);
			ActiveCondition.Time_active = 1;
			ActiveCondition.time_set_index = i;
			ActiveCondition.pCondition = pCondition;
		}

	}

	// second, if we are in the active time proid, detetive if in the zone

	if(ActiveCondition.Time_active == 1){

		if(pGPS->speed > pCondition->speed){	// exceed the speed limit

			ActiveCondition.active = 1;
		} else {

			ActiveCondition.active = 0;
		}
	}

}


static int Speed_Interval_Check(SpeedUpdate_Info *pActiveCondition)
{
	int 		need_send;
	time_t 		current_time;
	uint16_t 	interval;

	need_send = 0;

	if(pActiveCondition->active==1){

		if(pActiveCondition->SendCount==0){
			need_send = 1;
		} else {

			time(&current_time);
			interval = pActiveCondition->pCondition->Info.Interval;

			if( (current_time - pActiveCondition->lastTime) > interval ) {
				need_send = 1;
				pActiveCondition->lastTime = current_time;
			}
		}
	}

	return need_send;
}


int SpeedReport_Update(const GpsInfo *pGPS)
{
	int res;
	int data_len;

	if( 1 == Speed_Interval_Check(&ActiveCondition) ){

		DEBUG_MSG("%s : > Update the speed msg...\n", __func__);
		Speed_AKV_Enpacket(0x0008, pGPS, ActiveCondition.pCondition, Speed_Data_Buff, &data_len);
		res = ReportUpdata_Init(Speed_Data_Buff, data_len);
		ActiveCondition.SendCount++;
	}

	return res;
}

static void Speed_AKV_Enpacket(const int action, const GpsInfo *pGPS, const SpeedAnomaly_Report *pCondition, char *pbuff, int *len)
{
	char *pData;
	
	int  i;
	int  id_len;
	char *pId_str;

	int  pI_AKV_len;
	int  speed;
	int  direction;

	id_len  = pCondition->Info.R_len;
	pId_str = (char *)pCondition->Info.RID;

	speed = (int)(pGPS->speed);
	direction = (int)(pGPS->direction);

	pData = pbuff;

	// rname AKV
	*(pData++) = 0x00;		// attr
	*(pData++) = 0x05;		// key len

	*(pData++) = 'r';		// key
	*(pData++) = 'n'; 
	*(pData++) = 'a';
	*(pData++) = 'm'; 
	*(pData++) = 'e';

	*(pData++) = 0x00;		// value len
	*(pData++) = 0x02;

	*(pData++) = (char)(action >> 8);	// value
	*(pData++) = (char)(action);

	// rid AKV 
	*(pData++) = 0x00;					// attr
	*(pData++) = 0x03;					// key len

	*(pData++) = 'r';					// key
	*(pData++) = 'i'; 
	*(pData++) = 'd';

	*(pData++) = (char)(id_len >> 8);	// value len
	*(pData++) = (char)(id_len);
	
										// value
	for(i=0; i<id_len; i++){

		*(pData++) = *(pId_str++);
	}

	// p AKV
	*(pData++) = 0x00;		// attr
	*(pData++) = 0x01;		// key len

	*(pData++) = 'p';		// key

	*(pData++) = 0x00;		// value len
	*(pData++) = 0x01;

	*(pData++) = pCondition->Info.Priority;	// value

	// pI AVK
	pI_AKV_Enpacket(pGPS, pData, &pI_AKV_len);
	pData = pData + pI_AKV_len;

	// s AKV
	*(pData++) = 0x00;		// attr
	*(pData++) = 0x01;		// key len

	*(pData++) = 's';		// key

	*(pData++) = 0x00;		// value len
	*(pData++) = 0x02;

	*(pData++) = (char)(speed>>8);
	*(pData++) = (char)(speed);

	//d AKV
	*(pData++) = 0x00;		// attr
	*(pData++) = 0x01;		// key len

	*(pData++) = 'd';		// key

	*(pData++) = 0x00;		// value len
	*(pData++) = 0x02;

	*(pData++) = (char)(direction>>8);
	*(pData++) = (char)(direction);

	*len = (pData - pbuff);
}
