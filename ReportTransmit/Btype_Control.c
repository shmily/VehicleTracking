/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-10-18 14:23
#      Filename : Btype_Control.c
#   Description : not thing...
#
=============================================================================*/
#include "UDP_Lib.h"
#include "GSM_Hal.h"
#include "GSM-error.h"
#include "TLP.h"
#include "AppLayer.h"

#include "Btype_Control.h"
#include "Btype_Transmit.h"

#include <string.h>
#include <stdio.h>
#include <unistd.h>   // for pause()  
#include <signal.h>   // for signal()  
#include <sys/time.h> // struct itimeral. setitimer()  

static Btype_State		Comunicate_State;
static char				Btype_APP_BUFF[256];
static int 				APP_DataLen;

Btype_State Btype_GetCurrentState(void)
{
	return Comunicate_State;
}


void Btype_SetState(Btype_State state)
{
	Comunicate_State = state;
}


int ReportUpdata_Init(const void *pdata_src, int len)
{

	APP_Error res;

	if(Comunicate_State==Btype_Idle){

		memcpy((char *)Btype_APP_BUFF, pdata_src, len);
		APP_DataLen = len;

		res = Btype_SendRequest(Btype_APP_BUFF, APP_DataLen);

		if(res==APP_ERR_NONE){
			Comunicate_State = Btype_REQ_SEND;
			fprintf(stderr, "%s : > Request send ...\n", __func__);
			return 0;
		} else {
			fprintf(stderr, "%s : > GSM mode err ...\n", __func__);
			return -1;
		}

	} else {
		fprintf(stderr, "%s : > B type transmit busy ...\n", __func__);
		return -1;
	}
}


int ReportUpdata_Loop(int flag)
{
	if(Comunicate_State==Btype_Packet_ACK_OK){

		fprintf(stderr, "%s : > Server had receive the packet ...\n", __func__);	
		Comunicate_State = Btype_Idle;
		return 0;
	} else {
		Comunicate_State = Btype_HAL_ERR;
		fprintf(stderr, "%s : > Packet ACK Error ...\n", __func__);
		return -1;
	}
}
