/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-10-18 14:24
#      Filename : Btype_InputHandle.c
#   Description : not thing...
#
=============================================================================*/
#include <stdint.h>
#include <math.h>
#include "UDP_Lib.h"
#include "GSM_Hal.h"
#include "GSM-error.h"
#include "TLP.h"
#include "AppLayer.h"
#include "crc16.h"

#include "Btype_InputHandle.h"
#include "Btype_Control.h"

#include <assert.h>
#include <time.h>
#include <errno.h>
#include <string.h>

static char BT_InputBuff[256];


static void Btype_PacketACK(char *pdata, char *pACK);


void Btype_InputHandle(const void *pdata, int length)
{
	TLP_Head_Struct *pHead;
	char			*pReceive;
	char			ack_error;

	assert(length<256);
	memcpy((char *)BT_InputBuff, pdata, length);

	pHead    = (TLP_Head_Struct *)BT_InputBuff;
	pReceive = (char *)(BT_InputBuff + sizeof(TLP_Head_Struct));

	switch(pHead->type){

		case _Btype_Packet_ACK:

			DEBUG("%s : > Packet ACK ...\n", __func__);
			Btype_PacketACK(pReceive, &ack_error);	
			DEBUG("%s : > Error code = 0x%02X\n", __func__, ack_error);
		
			if(Btype_GetCurrentState()==Btype_REQ_SEND){

				if(ack_error==_Btype_Tx_Success){
					Btype_SetState(Btype_Packet_ACK_OK);
				} else {
					Btype_SetState(Btype_Packet_ACK_ERR);
				}
			}

			break;

		default :

			DEBUG("%s : > Unknow packet ...\n", __func__);
			
			break;
	}

}


static void Btype_PacketACK(char *pdata, char *pACK)
{
	(*pACK) = (*pdata);
}
