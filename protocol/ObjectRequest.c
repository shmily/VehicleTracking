
#include "ObjectRequest.h"
#include "UDP_Lib.h"
#include "crc16.h"
#include "SequenceNumber.h"
#include "Timestamp.h"
#include "Task.h"

#define	_OR_Packet_Len_		512

uint8_t		OR_Packet[_OR_Packet_Len_];

static APP_Error HeartBeatHandle(CommandType *command);

APP_Error ObjectRequestHandle(CommandType *command)
{
	APP_Error res;
	if(command->Action == _HeartBeat){
		DEBUG("%s : This is a heart beat packet...\n",__func__);
		res = HeartBeatHandle(command);
	} else {
		DEBUG("%s : This is a unknow packet...\n",__func__);
		res = APP_ERR_UNKNOW;
	}
	
	return res;
}

static APP_Error HeartBeatHandle(CommandType *command)
{
	DEBUG("%s : receive heart beat ack packet...\n",__func__);

	ClearTimeOut(_Task_RE_Login_);	
	return	APP_ERR_NONE;
}


// Heart Beat requeset
APP_Error HeartBeat_Request(TLP_Head_Struct *pHeadInfo)
{
	GSM_Error		res;

	uint32_t		timestamp;
	Packet_Struct   Packet;
	uint16_t		crc_resault;
	TLP_Head_Struct	*PacketHead;
	uint8_t			*pData;

	PacketHead = (TLP_Head_Struct *)&(Packet.Data[0]);
	pData      = (uint8_t *)&(Packet.Data[0]) + sizeof(TLP_Head_Struct);
	
	// no need to change
	PacketHead->version = _VERSION;
	PacketHead->length  = sizeof(TLP_Head_Struct);
	PacketHead->type    = _TransmitCtrl;			// device to server
	PacketHead->medium  = _TransmitType;			// UDP
	PacketHead->encrypt = _Encrypt;					// encrypt none
	PacketHead->reserve = _Reserve;			
	
	PacketHead->DEV_IDH = HTONL(_DEV_IDH);
	PacketHead->DEV_IDL = HTONL(_DEV_IDL);
	
	//need to change
	GetTimestamp(&timestamp);
	PacketHead->timestamp = HTONL(timestamp);		// need to fix
	PacketHead->SEQ_num   = HTONL(Get_SEQ());
	
	// send the ack
	*(pData++) = HEART_BEAT;						// command type
	*(pData++) = (uint8_t)(_HeartBeat>>8);
	*(pData++) = (uint8_t)(_HeartBeat);				// action

	Packet.length = pData - ((uint8_t *)&(Packet.Data[0]));

	// get the crc16
	pData         = (uint8_t *)&(Packet.Data[0]) + 2;	
	crc_resault   = Caculate(pData, (Packet.length-2));
	PacketHead->crc16 = HTONS(crc_resault);
	
	// send packet
	res = UDP_SendPacket((char *)&(Packet.Data[0]), Packet.length);
	
	if(res!=ERR_NONE){
		DEBUG("%s : ACK ...Error!\n",__func__);
		return APP_ERR_SEND;
	} else {
		DEBUG("%s : ACK ...OK\n",__func__);
		return APP_ERR_NONE;
	}
}
