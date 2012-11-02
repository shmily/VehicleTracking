/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-10-18 13:58
#      Filename : TLP.c
#   Description : not thing...
#
=============================================================================*/

#include "TLP.h"
#include "crc16.h"
#include "DtypeTransmit.h"
#include "Btype_InputHandle.h"

// packet devide
TLP_Error TLP_PacketDevide(Packet_Struct *TLP_Pack, PacketInfo *APP_Packet, int *packet_type)
{
	TLP_Head_Struct *pTlpHead;
	uint32_t		len;
	uint16_t		crc_i;
	int				i;

	*packet_type = TypeX;

	pTlpHead = (TLP_Head_Struct *)(&(TLP_Pack->Data[0]));
	len      = TLP_Pack->length;
	
	
	// check crc
	crc_i = Caculate(&(TLP_Pack->Data[2]),len-2)&0xFFFF;
	pTlpHead->crc16 = HTONS(pTlpHead->crc16);	

	if(crc_i != pTlpHead->crc16){
		DEBUG("%s : crc error!\n",__func__);
		return ERROR_CRC;
	} else {
	
		switch (pTlpHead->type & 0x70)
		{
			case TypeA :	// type a packet 	 
				DEBUG("%s : This is a Type A transmit packet...\n",__func__);
				APP_Packet->TransmitType = pTlpHead->type;
				APP_Packet->SEQ_num = pTlpHead->SEQ_num;
				APP_Packet->DatPointer = &(TLP_Pack->Data[24]);
				APP_Packet->length = len - 24;
				*packet_type = TypeA;
				break;
	
			case TypeD :	// type a packet 	 
				DEBUG("%s : This is a Type D transmit packet...\n",__func__);	
				DT_InputPacketHandle((char *)&(TLP_Pack->Data[0]),len);	
				*packet_type = TypeD;
				break;

			case TypeB :
				DEBUG("%s : This is a Type B transmit packet...\n",__func__);	
				Btype_InputHandle((char *)&(TLP_Pack->Data[0]),len);
				*packet_type = TypeB;
				break;
	
			default :
				*packet_type = TypeX;
				return ERROR_UNKNOW;
		}
		
	}

	for(i=0;i<len-24;i++){
		DEBUG("0x%02X ", TLP_Pack->Data[24+i]);
	}
	DEBUG("\n\n");
	return ERROR_NONE;
}
