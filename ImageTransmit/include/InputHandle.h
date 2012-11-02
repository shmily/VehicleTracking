/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-10-18 10:06
#      Filename : InputHandle.h
#   Description : not thing...
#
=============================================================================*/

#ifndef _DT_INPUT_HANDLE_H_
#define _DT_INPUT_HANDLE_H_

#include <stdio.h>
#include "GSM-error.h"
#include "ImageTransmit.h"

typedef struct _Retry_Info
{
	char 	error_code;
	char	packet_num;
	int		packetIndex[16];
}Retry_Info;

DT_StateFlag Get_DT_State(void);
void DT_InputPacketHandle(const void *pData, int length);
void Set_DT_State(DT_StateFlag state);
#endif





