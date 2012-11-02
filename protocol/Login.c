
#include "Login.h"


GSM_Error Send_LoginPacket(void)
{
	GSM_Error		res;
	Packet_Struct   Packet;
	uint16_t		crc_resault;
	TLP_Head_Struct	*PacketHead;
	uint8_t			*pData;
	
	int				i;
	uint32_t		timestamp;

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
	PacketHead->SEQ_num = HTONL(Get_SEQ());
	
	*(pData++) = 0x41;
	*(pData++) = 0x00;
	*(pData++) = 0x01;

	// prepare to send packet
	Packet.length = pData - ((uint8_t *)&(Packet.Data[0]));
	
	// get the crc16
	pData         = (uint8_t *)&(Packet.Data[0]) + 2;	
	crc_resault   = Caculate(pData, (Packet.length-2));
	PacketHead->crc16 = HTONS(crc_resault);	

	for(i=0;i<Packet.length;i++){
		fprintf(stderr,"0x%02X ", Packet.Data[i]);
	}
	fprintf(stderr,"\n\n");
	// send packet
	res = UDP_SendPacket((char *)&(Packet.Data[0]), Packet.length);
	
	if(res!=ERR_NONE){
		DEBUG("%s : Login Packet send ...Error!\n",__func__);
	} else {
		DEBUG("%s : Login Packet send ...OK\n",__func__);
	}
	
	return res;
}

APP_Error Wait_LoginACK(void *pdata)
{
	char 	*p;
	char 	type;
	int	 	action;
	char 	ack;
		
	p = (char *)pdata+sizeof(TLP_Head_Struct);
	
	type = *(p++);
	action = *(p++);
	action = ((action&0x00FF)<<8)+(*(p++));
	ack = *(p++);
	
	if((type==0x41)&&(action==0x0001)){
		if(ack==0x01){
			return APP_ERR_NONE;
		}else{
			DEBUG("%s : Login...ACK ERROR.\n",__func__);
		}
	}
	else{
		DEBUG("%s : This is not a Login Packet ACK.\n",__func__);
	}

	return APP_ERR_UNKNOW;
}


GSM_Error Send_RegisterPacket(void)
{
	GSM_Error		res;
	Packet_Struct   Packet;
	uint16_t		crc_resault;
	TLP_Head_Struct	*PacketHead;
	uint8_t			*pData;
	
	int				i;
	uint32_t		timestamp;

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
	PacketHead->SEQ_num = HTONL(Get_SEQ());
	
	*(pData++) = 0x41;
	*(pData++) = 0x00;
	*(pData++) = 0x00;
	
	// --- ver attr ---
	// attr
	*(pData++) = 0x00;
	// K_len
	*(pData++) = 0x03;
	// key
	*(pData++) = 'v';
	*(pData++) = 'e';
	*(pData++) = 'r';
	// V_len
	*(pData++) = 0x00;
	*(pData++) = 0x0b;
	// value
	*(pData++) = '1';
	*(pData++) = '2';
	*(pData++) = '8';
	*(pData++) = '0';
	*(pData++) = '0';
	*(pData++) = '0';
	*(pData++) = '3';
	*(pData++) = '0';
	*(pData++) = '1';
	*(pData++) = '0';
	*(pData++) = '0';

	// --- ven attr ---
	// attr
	*(pData++) = 0x00;
	// K_len
	*(pData++) = 0x03;
	// key
	*(pData++) = 'v';
	*(pData++) = 'e';
	*(pData++) = 'n';
	// V_len
	*(pData++) = 0x00;
	*(pData++) = 0x04;
	// value
	*(pData++) = '1';
	*(pData++) = '0';
	*(pData++) = '6';
	*(pData++) = '7';

	// --- group attr ---
	// attr
	*(pData++) = 0x00;
	// K_len
	*(pData++) = 0x05;
	// key
	*(pData++) = 'g';
	*(pData++) = 'r';
	*(pData++) = 'o';
	*(pData++) = 'u';
	*(pData++) = 'p';
	// V_len
	*(pData++) = 0x00;
	*(pData++) = 0x0b;
	// value
	*(pData++) = 'c';
	*(pData++) = 'l';
	*(pData++) = 'd';
	*(pData++) = 'w';
	*(pData++) = '0';
	*(pData++) = '0';
	*(pData++) = '0';
	*(pData++) = '0';
	*(pData++) = '0';
	*(pData++) = '6';
	*(pData++) = '9';

	// prepare to send packet
	Packet.length = pData - ((uint8_t *)&(Packet.Data[0]));
	
	// get the crc16
	pData         = (uint8_t *)&(Packet.Data[0]) + 2;	
	crc_resault   = Caculate(pData, (Packet.length-2));
	PacketHead->crc16 = HTONS(crc_resault);	

	fprintf(stderr,"\n\n");
	for(i=0;i<Packet.length;i++){
		fprintf(stderr,"0x%02X ", Packet.Data[i]);
	}
	fprintf(stderr,"\n\n");
	// send packet
	res = UDP_SendPacket((char *)&(Packet.Data[0]), Packet.length);
	
	if(res!=ERR_NONE){
		DEBUG("%s : Register Packet send ...Error!\n",__func__);
	} else {
		DEBUG("%s : Register Packet send ...OK\n",__func__);
	}
	
	return res;
}


APP_Error Wait_RegisterACK(void *pdata)
{
	char 	*p;
	char 	type;
	int	 	action;
	char 	ack;
	
	p = (char *)pdata+sizeof(TLP_Head_Struct);
	
	type = *(p++);
	action = *(p++);
	action = ((action&0x00FF)<<8)+(*(p++));
	ack = *(p++);
	
	if((type==0x41)&&(action==0x0000)){
		if(ack==0x01){
			return APP_ERR_NONE;
		}else{
			DEBUG("%s : Register...ACK ERROR.\n",__func__);
		}
	}
	else{
		DEBUG("%s : This is not a Register Packet ACK.\n",__func__);
	}

	return APP_ERR_UNKNOW;
}
