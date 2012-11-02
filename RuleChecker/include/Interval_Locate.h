/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-10-22 13:27
#      Filename : Interval_Locate.h
#   Description : not thing...
#
=============================================================================*/

#ifndef __INTERVAL_LOCATE_H__
#define __INTERVAL_LOCATE_H__

#include <time.h>

#include "GPS.h"

// position updata list
typedef struct _PositionUpdate_
{
	int 			active;
	int     		SendCount;
	time_t			lastTime;
	Rule_Struct 	*pRule;
}PositionUpdate_Info;


void pI_AKV_Enpacket(const GpsInfo *pGPS, char *pbuff, int *len);
void Interval_Locate_Init(void);
void Interval_Locate_Check(struct list_head *pHead);
int Interval_Locate_Updata(const GpsInfo *pGPS);


#endif
