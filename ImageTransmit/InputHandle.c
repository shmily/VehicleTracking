
#include <stdint.h>
#include <math.h>
#include "UDP_Lib.h"
#include "GSM_Hal.h"
#include "GSM-error.h"
#include "TLP.h"
#include "AppLayer.h"
#include "DtypeTransmit.h"
#include "crc16.h"

#include "Task.h"
#include "Timestamp.h"
#include "SequenceNumber.h"

#include <assert.h>
#include <time.h>
#include <errno.h>

static char 	DT_InputBuff[256];

Retry_Info		RetryPacket;

static void Printf_RtryPacket(Retry_Info *pInfo);
static void DT_UpdateReqACK(char *pData, char *pACK);
static void DT_PacketACK(char *pData, Retry_Info *pInfo);
static void DT_DisconnetcACK(char *pData, char *error_code);

void DT_InputPacketHandle(const void *pData, int length)
{
	TLP_Head_Struct *pHead;
	char  			*pReceive;
	char			ACK_error;
	
	assert(length < 256);
	memcpy((char *)DT_InputBuff, pData, length);
	
	pHead = (TLP_Head_Struct *)DT_InputBuff;
	pReceive = (char *)(DT_InputBuff + sizeof(TLP_Head_Struct));
	
	switch(pHead->type){

		case _DT_UPDATE_ACK:
			DEBUG("%s : > Connection REQ ACK...!\n",__func__);
			DT_UpdateReqACK(pReceive, &ACK_error);
			if(ACK_error==0x00){
				Set_DT_State(DT_ConnectREQ_OK);
			} else {
				Set_DT_State(DT_ConnectREQ_ERR);
			}
			
			break;
		
		case _DT_PACKET_ACK:
		case _DT_RSEND_ACK:
		case _DT_RCONNECT_ACK:
			DEBUG("%s : > Packet updata ACK...!\n",__func__);
			DT_PacketACK(pReceive, &RetryPacket);
			Set_DT_State(DT_Packet_ACK);
			Printf_RtryPacket(&RetryPacket);
			
			break;
		
		case  _DT_DISCONNECT_ACK:
			DEBUG("%s : > Disconnect REQ ACK...!\n",__func__);
			DT_DisconnetcACK(pReceive, &ACK_error);
			if(ACK_error==0x00){
				Set_DT_State(DT_DisconnectREQ_OK);
			} else {
				Set_DT_State(DT_DisconnectREQ_ERR);
			}
			
			break;
			
		default:
			DEBUG("%s : > Unknow packet...!\n",__func__);
			break;
	}
}

static void DT_UpdateReqACK(char *pData, char *pACK)
{
	(*pACK) = *(pData+16);
}


static void DT_PacketACK(char *pData, Retry_Info *pInfo)
{
		int i;
		int tmp;

		pData = pData + 16 + 2; 	
		pInfo->error_code = *(pData++);
		pInfo->packet_num = *(pData++);
	
		if(pInfo->error_code == 0x01){
			pInfo->packet_num = 0;
		} else if(pInfo->error_code == 0x02){
			for(i=0; i<pInfo->packet_num; i++){
			
				tmp = *(pData++);
				tmp = (tmp<<8) + *(pData++);
				pInfo->packetIndex[i] = tmp;
			}
		} else if(pInfo->error_code == 0x03){
			pInfo->packet_num = 16;
		}
}


static void DT_DisconnetcACK(char *pData, char *error_code)
{	
	(*error_code) = *(pData);
}

static void Printf_RtryPacket(Retry_Info *pInfo)
{
	int i;
	
	fprintf(stderr, "\r\n>>\r\n");
	fprintf(stderr,"%s : > error_code    = 0x%02X\n", __func__, pInfo->error_code);
	fprintf(stderr,"%s : > packet_num    = 0x%02X\n", __func__, pInfo->packet_num);
	fprintf(stderr,"%s : > Resend Packet : ", __func__);
	for(i=0; i<pInfo->packet_num; i++){
		fprintf(stderr,"[%d] ", pInfo->packetIndex[i]);
	}
	fprintf(stderr, "\r\n");
}
