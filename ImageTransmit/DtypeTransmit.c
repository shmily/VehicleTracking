
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

#include <time.h>
#include <errno.h>

#define		_DT_PacketLen_		1024	
static char DT_BUFF[_DT_PacketLen_];

static	void TL_Head_Init(TLP_Head_Struct *pHead)
{
	// no need to change
	pHead->version = _VERSION;
	pHead->length  = sizeof(TLP_Head_Struct);
	pHead->type    = 0x80+0x10;				// device to server; typeB send data
	pHead->medium  = _TransmitType;			// UDP
	pHead->encrypt = _Encrypt;					// encrypt none
	pHead->reserve = _Reserve;			
	
	pHead->DEV_IDH = HTONL(_DEV_IDH);
	pHead->DEV_IDL = HTONL(_DEV_IDL);
}


APP_Error DT_UpdateRequest(Image_Struct *pImage)
{
	GSM_Error		res;
	TLP_Head_Struct	*PacketHead;
	char			*pData;
	int				length;
	uint32_t		timestamp;
	uint32_t		size;
	uint16_t		crc_resault;
	
	PacketHead = (TLP_Head_Struct *)DT_BUFF;
	pData	   = DT_BUFF + sizeof(TLP_Head_Struct);
	
	// Transmit layer hear
	TL_Head_Init(PacketHead);
	PacketHead->type = _DT_UPDATE;
	GetTimestamp(&timestamp);
	PacketHead->timestamp = HTONL(timestamp);
	PacketHead->SEQ_num = HTONL(Get_SEQ());
	
	// MD5
	memcpy(pData, &(pImage->MD5[0]), 16);
	
	// image size
	pData += 16;
	size = pImage->size;
	*(pData++) = (uint8_t)(size>>24);
	*(pData++) = (uint8_t)(size>>16);
	*(pData++) = (uint8_t)(size>>8);
	*(pData++) = (uint8_t)size;
	
	// packet size
	*(pData++) = (uint8_t)(_DT_SIZE >> 8);
	*(pData++) = (uint8_t)(_DT_SIZE);
	
	// file name
	memcpy(pData, &(pImage->name[0]), 6);
	
	// pip num
	pData += 6;
	*(pData++) = 0x01;		// we use the pip 0 only
	
	// type
	*(pData++) = pImage->type;
	
	// length
	length = pData - DT_BUFF;
	PacketHead->length = length;	// 这是有可选项的头
	
	// get crc
	pData         = DT_BUFF + 2;	
	crc_resault   = Caculate((const uint8_t *)pData, (length-2));
	PacketHead->crc16 = HTONS(crc_resault);	

	int i;
	fprintf(stderr,"%s : >\n",__func__);
	for(i=0;i<length;i++){
		fprintf(stderr,"0x%02X ", DT_BUFF[i]);
	}	
	fprintf(stderr,"\r\n");

	// send packet
	res = UDP_SendPacket((char *)DT_BUFF, length);
	
	if(res!=ERR_NONE){
		DEBUG("%s : updata request ...Error!\n",__func__);
		return APP_ERR_SEND;
	} else {
		DEBUG("%s : updata request ...OK\n",__func__);
		return APP_ERR_NONE;
	}
}


APP_Error DT_UpdatePacket(Image_Struct *pImage, DT_Packet *pPacket)
{

	GSM_Error		res;
	TLP_Head_Struct	*PacketHead;
	char			*pData;
	int				length;
	uint32_t		timestamp;
	uint16_t		num;
	uint16_t		crc_resault;

	PacketHead = (TLP_Head_Struct *)DT_BUFF;
	pData	   = DT_BUFF + sizeof(TLP_Head_Struct);
	
	// Transmit layer hear
	TL_Head_Init(PacketHead);
	PacketHead->type = _DT_PACKET;
	GetTimestamp(&timestamp);
	PacketHead->timestamp = HTONL(timestamp);
	PacketHead->SEQ_num = HTONL(Get_SEQ());
	
	// MD5
	memcpy(pData, &(pImage->MD5[0]), 16);
	
	// need ack
	pData = pData + 16;
	*(pData++) = pPacket->need_ack;
	
	//Packet num
	num = pPacket->num;
	*(pData++) = (uint8_t)(num>>8);
	*(pData++) = (uint8_t)(num);

	PacketHead->length = pData - DT_BUFF;	// 这是有可选项的头

	// packet data ---------------------
	memcpy(pData, (char *)(pPacket->pdata), pPacket->actual_len);
	pData = pData + pPacket->actual_len;
	
	// length
	length = pData - DT_BUFF;	
	DEBUG("%s : > length = %d\n",__func__,length);

	// get crc
	pData         = DT_BUFF + 2;	
	crc_resault   = Caculate((const uint8_t *)pData, (length-2));
	PacketHead->crc16 = HTONS(crc_resault);	

	int i;

	for(i=0; i<length; i++){
		DEBUG("0x%02X ", DT_BUFF[i]);
		
		if((i%16)==0)
			DEBUG("\n");
	}


	// send packet
	res = UDP_SendPacket((char *)DT_BUFF, length);	

	if(res!=ERR_NONE){
		DEBUG("%s : updata packet ...Error!\n",__func__);
		return APP_ERR_SEND;
	} else {
		DEBUG("%s : updata packet ...OK\n",__func__);
		return APP_ERR_NONE;
	}
}


APP_Error DT_RSendRequest(Image_Struct *pImage, int packet_num)
{
	GSM_Error		res;
	TLP_Head_Struct	*PacketHead;
	char			*pData;
	int				length;
	uint32_t		timestamp;
	uint32_t		size;
	uint16_t		crc_resault;
	
	PacketHead = (TLP_Head_Struct *)DT_BUFF;
	pData	   = DT_BUFF + sizeof(TLP_Head_Struct);
	
	// Transmit layer hear
	TL_Head_Init(PacketHead);
	PacketHead->type = _DT_RSEND;
	GetTimestamp(&timestamp);
	PacketHead->timestamp = HTONL(timestamp);
	PacketHead->SEQ_num = HTONL(Get_SEQ());
	
	// MD5
	memcpy(pData, &(pImage->MD5[0]), 16);
	
	// image size
	pData += 16;
	size = pImage->size;
	*(pData++) = (uint8_t)(size>>24);
	*(pData++) = (uint8_t)(size>>16);
	*(pData++) = (uint8_t)(size>>8);
	*(pData++) = (uint8_t)size;
	
	// packet num
	*(pData++) = (uint8_t)(packet_num >> 8);
	*(pData++) = (uint8_t)(packet_num);
	
	// length
	length = pData - DT_BUFF;	
	PacketHead->length = length;	// 这是有可选项的头

	// get crc
	pData         = DT_BUFF + 2;	
	crc_resault   = Caculate((const uint8_t *)pData, (length-2));
	PacketHead->crc16 = HTONS(crc_resault);	

	// send packet
	res = UDP_SendPacket((char *)DT_BUFF, length);
	
	if(res!=ERR_NONE){
		DEBUG("%s : Resend packet request ...Error!\n",__func__);
		return APP_ERR_SEND;
	} else {
		DEBUG("%s : Resend packet request ...OK\n",__func__);
		return APP_ERR_NONE;
	}
}


APP_Error DT_DisConnectRequest(char error_code)
{
	GSM_Error		res;
	TLP_Head_Struct	*PacketHead;
	char			*pData;
	int				length;
	uint32_t		timestamp;
	uint16_t		crc_resault;
	
	PacketHead = (TLP_Head_Struct *)DT_BUFF;
	pData	   = DT_BUFF + sizeof(TLP_Head_Struct);
	
	// Transmit layer hear
	TL_Head_Init(PacketHead);
	PacketHead->type = _DT_DISCONNECT;
	GetTimestamp(&timestamp);
	PacketHead->timestamp = HTONL(timestamp);
	PacketHead->SEQ_num = HTONL(Get_SEQ());	
	
	*(pData++) = error_code;
	
	// length
	length = pData - DT_BUFF;	
	PacketHead->length = length;	// 这是有可选项的头

	// get crc
	pData         = DT_BUFF + 2;	
	crc_resault   = Caculate((const uint8_t *)pData, (length-2));
	PacketHead->crc16 = HTONS(crc_resault);	

	int i;

	DEBUG("\n");
	for(i=0; i<length; i++){
		DEBUG("0x%02X ", DT_BUFF[i]);
		
		if((i%16)==0)
			DEBUG("\n");
	}
	DEBUG("\n");

	// send packet
	res = UDP_SendPacket((char *)DT_BUFF, length);
	
	if(res!=ERR_NONE){
		DEBUG("%s : Disconnect request ...Error!\n",__func__);
		return APP_ERR_SEND;
	} else {
		DEBUG("%s : Disconnect request ...OK\n",__func__);
		return APP_ERR_NONE;
	}	
}

