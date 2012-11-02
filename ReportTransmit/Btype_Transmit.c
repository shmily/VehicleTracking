/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-10-18 14:24
#      Filename : Btype_Transmit.c
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

#include "Task.h"
#include "Timestamp.h"
#include "SequenceNumber.h"

#include <time.h>
#include <errno.h>

#include "Btype_Transmit.h"

#define		_Btype_PacketLen	512

static	char Btype_BUFF[_Btype_PacketLen];

static void Btype_TL_Head_Init(TLP_Head_Struct *pHead)
{
	// no need to change
	pHead->version = _VERSION;
	pHead->length  = sizeof(TLP_Head_Struct);
	pHead->type    = 0x80+0x10;					// device to server; typeB send data
	pHead->medium  = _TransmitType;				// UDP
	pHead->encrypt = _Encrypt;					// encrypt none
	pHead->reserve = _Reserve;			
	
	pHead->DEV_IDH = HTONL(_DEV_IDH);
	pHead->DEV_IDL = HTONL(_DEV_IDL);
}

APP_Error Btype_SendRequest(const void *pdata_src, int length)
{
	GSM_Error		res;
	TLP_Head_Struct *PacketHead;
	char			*pData;
	int 			Packet_len;
	uint32_t		timestamp;
	uint16_t		crc_resault;

	PacketHead = (TLP_Head_Struct *)Btype_BUFF;
	pData	   = Btype_BUFF + sizeof(TLP_Head_Struct);
	
	// Transmit layer hear
	Btype_TL_Head_Init(PacketHead);
	PacketHead->type = (char)_Btype_REQ;
	GetTimestamp(&timestamp);
	PacketHead->timestamp = HTONL(timestamp);
	PacketHead->SEQ_num = HTONL(Get_SEQ());
	PacketHead->length = sizeof(TLP_Head_Struct);

	*(pData++) = 0x40;
	*(pData++) = 0x00;
	*(pData++) = 0x01;

	*(pData++) = 0x00;

	*(pData++) = 0x00;
	*(pData++) = 0x03;
	*(pData++) = 0x54;
	*(pData++) = 0x69;
	*(pData++) = 0x64;
	*(pData++) = 0x00;
	*(pData++) = 0x04;
	*(pData++) = 0x43;
	*(pData++) = 0x41;
	*(pData++) = 0x4C;
	*(pData++) = 0x4C;

	// get crc
	Packet_len = pData - Btype_BUFF;
	pData      = (char *)(Btype_BUFF + 2);

	crc_resault   = Caculate((const uint8_t *)pData, (Packet_len-2));
	PacketHead->crc16 = HTONS(crc_resault);

	int i;

	for(i=0; i<Packet_len; i++){
		fprintf(stderr,"0x%02X ", Btype_BUFF[i]);
	}

	// send packet
	res = UDP_SendPacket((char *)Btype_BUFF, Packet_len);

	if(res!=ERR_NONE){
		DEBUG("%s : updata request ...Error!\n",__func__);
		return APP_ERR_SEND;
	} else {
		DEBUG("%s : updata request ...OK\n",__func__);
		return APP_ERR_NONE;
	}
}


APP_Error Btype_SendPacket (const void *pdata_src, int length)
{
	GSM_Error		res;
	TLP_Head_Struct	*PacketHead;
	char			*pData;
	int				packet_len;
	uint32_t		timestamp;
	uint16_t		crc_resault;

	PacketHead = (TLP_Head_Struct *)Btype_BUFF;
	pData	   = Btype_BUFF + sizeof(TLP_Head_Struct);
	
	// Transmit layer hear
	Btype_TL_Head_Init(PacketHead);
	PacketHead->type = (char)_Btype_Packet;
	GetTimestamp(&timestamp);
	PacketHead->timestamp = HTONL(timestamp);
	PacketHead->SEQ_num = HTONL(Get_SEQ());
	PacketHead->length = sizeof(TLP_Head_Struct);

	// app data
	memcpy(pData, (char *)pdata_src, length);

	pData = pData + length;

	// get crc
	packet_len = pData - Btype_BUFF;
	pData 	   = (char *)(Btype_BUFF + 2);

	crc_resault   = Caculate((const uint8_t *)pData, (packet_len-2));
	PacketHead->crc16 = HTONS(crc_resault);	

	// send packet
	res = UDP_SendPacket((char *)Btype_BUFF, packet_len);
	
	if(res!=ERR_NONE){
		DEBUG("%s : updata packet ...Error!\n",__func__);
		return APP_ERR_SEND;
	} else {
		DEBUG("%s : updata packet ...OK\n",__func__);
		return APP_ERR_NONE;
	}
}
