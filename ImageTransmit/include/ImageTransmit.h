/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-10-18 10:06
#      Filename : ImageTransmit.h
#   Description : not thing...
#
=============================================================================*/

#ifndef _DT_STATE_MACHINE_H_
#define _DT_STATE_MACHINE_H_

#include <stdio.h>
#include "GSM-error.h"


#define		_TimeOut_Flag		0x01
#define		_Receive_Flag		0x02

typedef enum  _DT_StateFlag
{
	
	DT_Idle,
	DT_Ready,
	
	DT_ConnectREQ_SEND,
	DT_ConnectREQ_OK,
	DT_ConnectREQ_ERR,
	
	DT_Packet_SEND,
	DT_Packet_ACK,
	
	DT_PacketCheck,
	DT_PacketCheck_ACK,
	
	DT_DisconnectREQ_OK,
	DT_DisconnectREQ_ERR,
	
	DT_File_ERR,
	DT_File_OK,
	
	DT_Image_Over,

	DT_SendError,
	
	DT_ConnetcReqError,
	DT_DisconnetcReqError
}DT_StateFlag;

int ImageTransmit_init(const char *path, const char *name);
int ImageTransmit_loop(int flag);


DT_StateFlag Get_DT_State(void);
void Set_DT_State(DT_StateFlag state);

#endif





