
#include "DeviceControl.h"
#include "UDP_Lib.h"
#include "crc16.h"

uint8_t OutPacket[1024] = {0};

static APP_Error RollCallHandle(CommandType *command);

APP_Error DeviceControlHandle(CommandType *command)
{
	APP_Error res;
	if(command->Action == ACTION_ROLLCALL){
		DEBUG("%s : This is a call the roll packet...\n",__func__);
		res = RollCallHandle(command);
	} else {
		DEBUG("%s : This is a unknow packet...\n",__func__);
		res = APP_ERR_UNKNOW;
	}
	
	return res;
}

static APP_Error RollCallHandle(CommandType *command)
{
	GSM_Error		res;
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
	PacketHead->timestamp = 0x00;					// need to fix
	PacketHead->SEQ_num = command->plowLevelInfo->SEQ_num;

	// send the ack
	*(pData++) = DEV_CONTROL;						// command type
	*(pData++) = 0x00;
	*(pData++) = 0x02;								// action
	*(pData++) = 0x01;								// success : 0x01; faild : 0x00
	
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
