/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-10-25 19:11
#      Filename : RegionDetection.c
#   Description : not thing...
#
=============================================================================*/

#include <time.h>
#include <stdint.h>
#include <string.h>

#include "RegionDetection.h"
#include "Debug.h"
#include "TimeCompare.h"
#include "Interval_Locate.h"

#include "CircleJudge.h"
#include "RectJudge.h"
#include "PolyJudge.h"
#include "GPS.h"
#include "ZoneInfo.h"
#include "Btype_Control.h"
#include "myList.h"
#include "ReportConditionList.h"
#include "ReportCondition.h"

static char					RDR_Data_Buff[256];
static RDR_Update_Info  	InZone_ActiveRule;
static RDR_Update_Info  	OutZone_ActiveRule;

static int  Is_InZone(char shape, void *pShape_Value, GpsInfo *pGps);
static void Condition_Check(struct list_head *pHead, const GpsInfo *pGPS, RDR_Update_Info *pActiveCondition, int inzone);
static void Report_AKV_Enpacket(const int action, const GpsInfo *pGPS, const InZone_Report_Struct *pCondition, char *pbuff, int *len);

void ZoneInfo_Report_Init(void)
{
	memset((char *)&InZone_ActiveRule,  0, sizeof(RDR_Update_Info));
	memset((char *)&OutZone_ActiveRule, 0, sizeof(RDR_Update_Info));
}

void ZoneInfo_Condition_Check(struct list_head *pHead_InZone, struct list_head *pHead_OutZone, const GpsInfo *pGPS)
{
	Condition_Check(pHead_InZone,  pGPS, &InZone_ActiveRule,  1);	
	Condition_Check(pHead_OutZone, pGPS, &OutZone_ActiveRule, 0);	
}

// 
// inzone : 1 --> in zone detect
// inzone : 0 --> out zone detect
//
static void Condition_Check(struct list_head *pHead, const GpsInfo *pGPS, RDR_Update_Info *pActiveCondition, int inzone)
{
	struct list_head 		*plist;
	InZone_Report_Struct 	*pCondition;
	Time_D					*pTime;
	
	int						mach = 0;
	int 					TimeSet_sum;
	int 					i;
	TimeSlot_Struct			*pTimeSlot;

	char					Zone_Shape;

	// first, we find a active time proid

	if(pActiveCondition->Time_active == 1){				// we aleady have a condition, we check it
		
		pCondition = pActiveCondition->pCondition;
		pTime = (Time_D *)&((pCondition->TimeSet).BE_Time[(pActiveCondition->time_set_index)].Etime);

		if( Is_BeyondTime_D( pTime ) == 1 ){				// out of the time


			DEBUG_MSG("%s : >>>> Delete the rule...\n", __func__);
			pActiveCondition->active = 0;
			pActiveCondition->Time_active = 0;
			pActiveCondition->SendCount = 0;

			ReportCondition_Del(&pCondition->list);
			free(pCondition->pShape_Value);
			free(pCondition);

			pCondition = NULL;
			pActiveCondition->pCondition = NULL;
		}

	} else {	// else, we search for a active condition

		list_for_each(plist, pHead){

			pCondition = list_entry(plist, InZone_Report_Struct, list);

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
			pActiveCondition->Time_active = 1;
			pActiveCondition->time_set_index = i;
			pActiveCondition->pCondition = pCondition;
		}
	}

	// second, if we are in the active time proid, detetive if in the zone

	if(pActiveCondition->Time_active == 1){

		Zone_Shape = pCondition->Shape;

		if(Is_InZone(Zone_Shape, pCondition->pShape_Value, (GpsInfo *)pGPS) == 1){

			pActiveCondition->active = ( (inzone==1) ? 1 : 0 );
		} else {

			pActiveCondition->active = ( (inzone==1) ? 0 : 1 );
		}
	}
}

static int Is_InZone(char shape, void *pShape_Value, GpsInfo *pGps)
{
	int res;

	switch (shape){

		case 0x01 :
			res = IsPosIn_Circle((Circle_Struct *)pShape_Value, pGps);
			break;

		case 0x02 :
			res = IsPosIn_Rect((Rectangle_Struct *)pShape_Value, pGps);
			break;

		case 0x03 :
			res = IsPosIn_Poly((Polygon_Struct *)pShape_Value, pGps);
			break;

		default :
			DEBUG_MSG("%s : > unknow shape ...\n", __func__);
			res = 0;
			break;
	}

	return res;
}


static int Report_Interval_Check(RDR_Update_Info *pActiveCondition)
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

int ZoneInfo_Update(const GpsInfo *pGPS)
{
	int res;
	int data_len;

	if( 1 == Report_Interval_Check(&InZone_ActiveRule) ){

		DEBUG_MSG("%s : > Update the InZone msg...\n", __func__);
		Report_AKV_Enpacket(0x0022, pGPS, InZone_ActiveRule.pCondition, RDR_Data_Buff, &data_len);
		res = ReportUpdata_Init(RDR_Data_Buff, data_len);
		InZone_ActiveRule.SendCount++;

	} else if( 1 == Report_Interval_Check(&OutZone_ActiveRule) ) {

		DEBUG_MSG("%s : > Update the OutZone msg...\n", __func__);
		Report_AKV_Enpacket(0x0023, pGPS, OutZone_ActiveRule.pCondition, RDR_Data_Buff, &data_len);
		res = ReportUpdata_Init(RDR_Data_Buff, data_len);
		OutZone_ActiveRule.SendCount++;
	}

	return res;
}

static void Report_AKV_Enpacket(const int action, const GpsInfo *pGPS, const InZone_Report_Struct *pCondition, char *pbuff, int *len)
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
