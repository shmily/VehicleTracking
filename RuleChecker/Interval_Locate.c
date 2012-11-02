/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-10-21 11:09
#      Filename : Interval_Locate.c
#   Description : not thing...
#
=============================================================================*/

#include <time.h>
#include <stdint.h>
#include <string.h>

#include "Debug.h"
#include "TimeCompare.h"

#include "ZoneInfo.h"
#include "Btype_Control.h"
#include "myList.h"
#include "RuleList.h"
#include "PositionUpdateRule.h"
#include "Interval_Locate.h"

extern int					test_rule;
static char					Position_Data_Buff[256];
static PositionUpdate_Info  ActiveRule;

static void Position_AKV_Enpacket(const char *tid, const GpsInfo *pGPS, char *pbuff, int *len);

void Interval_Locate_Init(void)
{
	memset((char *)&ActiveRule, 0, sizeof(PositionUpdate_Info));
}

void Interval_Locate_Check(struct list_head *pHead)
{
	struct list_head 	*plist;
	Rule_Struct 		*pRule;

	if(ActiveRule.active == 1){		// we aleady have a rule, we check it

		pRule = ActiveRule.pRule;

		if( Is_BeyondTime( (Time_DT *)(pRule->EMAP) ) == 1 ){	// out of the time


			DEBUG_MSG("%s : >>>> Delete the rule...\n", __func__);
			ActiveRule.active = 0;
			ActiveRule.SendCount = 0;

			Rule_Del(&pRule->list);
			free(pRule->BMAP);
			free(pRule->TMAP);
			free(pRule->EMAP);
			free(pRule);

			pRule = NULL;
			ActiveRule.pRule = NULL;

			test_rule = 1;
		}

	} else {	// else, we search for a active rule

		list_for_each(plist, pHead){

			pRule = list_entry(plist, Rule_Struct, list);

			if( (pRule->BMAP_Type == '6') && 
				(pRule->TMAP_Type == '7') && 
				(pRule->EMAP_Type == '6') ) {

				if( (Is_BeyondTime( (Time_DT *)(pRule->BMAP) ) == 1) && 
					(Is_BeyondTime( (Time_DT *)(pRule->EMAP) ) == 0) ) {

					break;
				}
			}
		}

		if(plist!=pHead){	// we get the rule

			DEBUG_MSG("%s : > we get the rule...\n", __func__);
			ActiveRule.active = 1;
			ActiveRule.pRule = pRule;
		}
	}
}


int Interval_Locate_Updata(const GpsInfo *pGPS)
{
	int 		res = 0;
	int     	data_len;
	int 		need_send;
	time_t 		current_time;
	uint16_t 	interval;

	need_send = 0;

	if(ActiveRule.active==1){

		if(ActiveRule.SendCount == 0){
			need_send = 1;
		} else {

			time(&current_time);
			interval = *((uint16_t *)(ActiveRule.pRule->TMAP));
			if((current_time-ActiveRule.lastTime) > interval) {
				need_send = 1;
				ActiveRule.lastTime = current_time;
			}
		}
	}

	if(need_send==1){		// update the packet

		DEBUG_MSG("%s : > Update the position msg...\n", __func__);
		Position_AKV_Enpacket("PRST", pGPS, Position_Data_Buff, &data_len);
		res = ReportUpdata_Init(Position_Data_Buff, data_len);

		ActiveRule.SendCount++;
	}

	return res;
}


void pI_AKV_Enpacket(const GpsInfo *pGPS, char *pbuff, int *len)
{
	char *pData;

	int  lat;
	int  lon;

	lat = NDEG2MSEC(pGPS->lat);
	lon = NDEG2MSEC(pGPS->lon);

	pData = pbuff;

	*(pData++) = 0x10;		// attr
	*(pData++) = 0x02;		// key len

	*(pData++) = 'p';		// key
	*(pData++) = 'I';

	*(pData++) = 0x00;		// value len
	*(pData++) = 0x25;

	// -- t akv
	*(pData++) = 0x00;		// attr
	*(pData++) = 0x01;		// key len

	*(pData++) = 't';		// key

	*(pData++) = 0x00;		// value len
	*(pData++) = 0x01;

	*(pData++) = 0x20;		// value ------------ need to be fix !!!

	// -- lat akv
	*(pData++) = 0x00;		// attr
	*(pData++) = 0x03;		// key len

	*(pData++) = 'l';		// key
	*(pData++) = 'a';
	*(pData++) = 't';

	*(pData++) = 0x00;		// value len
	*(pData++) = 0x04;

	*(pData++) = (char)(lat>>24);		// value
	*(pData++) = (char)(lat>>16);
	*(pData++) = (char)(lat>>8);
	*(pData++) = (char)(lat);

	// -- lon akv
	*(pData++) = 0x00;		// attr
	*(pData++) = 0x03;		// key len

	*(pData++) = 'l';		// key
	*(pData++) = 'o';
	*(pData++) = 'n';

	*(pData++) = 0x00;		// value len
	*(pData++) = 0x04;

	*(pData++) = (char)(lon>>24);		// value
	*(pData++) = (char)(lon>>16);
	*(pData++) = (char)(lon>>8);
	*(pData++) = (char)(lon);

	// -- alt akv
	*(pData++) = 0x00;		// attr
	*(pData++) = 0x03;		// key len

	*(pData++) = 'a';		// key
	*(pData++) = 'l';
	*(pData++) = 't';

	*(pData++) = 0x00;		// value len
	*(pData++) = 0x02;

	*(pData++) = 0x00;		// value ------------ need to be fix !!!
	*(pData++) = 0x0a;

	*len = pData - pbuff;
}


static void Position_AKV_Enpacket(const char *tid, const GpsInfo *pGPS, char *pbuff, int *len)
{
	char *pData;
	int  pI_AKV_len;
	int  speed;
	int  direction;

	speed = (int)(pGPS->speed);
	direction = (int)(pGPS->direction);

	pData = pbuff;

	// Tid AKV
	*(pData++) = 0x00;		// attr
	*(pData++) = 0x03;		// key len

	*(pData++) = 'T';		// key
	*(pData++) = 'i'; 
	*(pData++) = 'd';

	*(pData++) = 0x00;		// value len
	*(pData++) = 0x04;

	*(pData++) = *(tid++);	// value
	*(pData++) = *(tid++);
	*(pData++) = *(tid++);
	*(pData++) = *(tid++);

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
