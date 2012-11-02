
#include "crc16.h"
#include "PositionUpload.h"
#include <string.h>

#define		_UpLoadPacketLen_		512	

uint8_t Upload_Data[_UpLoadPacketLen_];


#ifdef	_EN_EX_AKV_
static void UploadPacket_Set_eX(uint8_t *pdata_start, Peripheral_info *info, uint8_t **pdata_end);
#endif
static void UploadPacket_Set_Type(uint8_t *pdata_start, uint8_t *pType, uint8_t **pdata_end);
static void En_PU_Packet(uint8_t *pdata_start, TLP_Head_Struct *pHeader, PostionUpload_info *pPU_info, int *plength);

APP_Error PositionUpload(PostionUpload_info *pInfo, Rule_Effective *pTry_info)
{
	GSM_Error		res;
	TLP_Head_Struct head;
	int				length;
	char			error_code;

	head.SEQ_num = 0x00000000;
	En_PU_Packet(Upload_Data, &head, pInfo, &length);

	res = Btype_SendPacket((char *)Upload_Data, length, pTry_info->Cnt, pTry_info->Interval*1000ul, &error_code);

	if(res!=ERR_NONE){
		DEBUG("%s : PositionUpload ...Error!\n",__func__);
		DEBUG("ERROR Code : 0x%02X\n", error_code);
		return APP_ERR_SEND;
	} else {
		DEBUG("%s : PositionUpload ...OK\n",__func__);
		return APP_ERR_NONE;
	}
}


static void En_PU_Packet(uint8_t *pdata_start, TLP_Head_Struct *pHeader, PostionUpload_info *pPU_info, int *plength)
{
	uint16_t		crc_resault;
	int				len;
	TLP_Head_Struct	*PacketHead;
	uint8_t			*pData;
	uint8_t			*pdata_end;

	memset(Upload_Data, 0, _UpLoadPacketLen_);		// clean it

	PacketHead = (TLP_Head_Struct *)pdata_start;
	pData      = (uint8_t *)pdata_start + sizeof(TLP_Head_Struct);

	// no need to change
	PacketHead->version = _VERSION;
	PacketHead->length  = sizeof(TLP_Head_Struct);
	PacketHead->type    = 0x80+0x10;				// device to server; typeB send data
	PacketHead->medium  = _TransmitType;			// UDP
	PacketHead->encrypt = _Encrypt;					// encrypt none
	PacketHead->reserve = _Reserve;			
	
	PacketHead->DEV_IDH = HTONL(_DEV_IDH);
	PacketHead->DEV_IDL = HTONL(_DEV_IDL);
	
	// need to change
	PacketHead->timestamp = 0x00;					// need to fix
	PacketHead->SEQ_num = HTONL(pHeader->SEQ_num);	// need to fix

	// packet data
	*(pData++) = UPLOAD_PACKET;						// upload packet
	*(pData++) = 0x00;
	*(pData++) = 0x01;								// position upload

	// add need return
	UploadPacket_Set_NeedReturn(pData, pPU_info->need_return, &pdata_end);

	// add packet type
	pData = pdata_end;
	UploadPacket_Set_Type(pData, pPU_info->pType, &pdata_end);

	// add position
	pData = pdata_end;	
	UploadPacket_Set_Position(pData, &(pPU_info->position), &pdata_end);

	// add speed
	pData = pdata_end;
	UploadPacket_Set_Speed(pData, pPU_info->speed, &pdata_end);

	// add direction
	pData = pdata_end;
	UploadPacket_Set_Direction(pData, pPU_info->direction, &pdata_end);

	// add mileage
	pData = pdata_end;
	UploadPacket_Set_Mileage(pData, pPU_info->mileage, &pdata_end);	

#ifdef	_EN_EX_AKV_
	// add ex akv
	pData = pdata_end;
	UploadPacket_Set_eX(pData, &(pPU_info->Oil_info), &pdata_end);		
#endif

	// add TSMAP AKV
	pData = pdata_end;
	UploadPacket_Set_TSMAP(pData, &(pPU_info->car_info), &pdata_end);	

	// get data length 
	pData = pdata_end;
	len = pData - pdata_start;

	// get the crc16
	pData         = pdata_start + 2;	
	crc_resault   = Caculate(pData, (len-2));
	PacketHead->crc16 = HTONS(crc_resault);

	(*plength) = len;
}


void UploadPacket_Set_NeedReturn(uint8_t *pdata_start, uint8_t flag, uint8_t **pdata_end)
{
	(*pdata_start++) = flag;
	(*pdata_end) = pdata_start;
}

static void UploadPacket_Set_Type(uint8_t *pdata_start, uint8_t *pType, uint8_t **pdata_end)
{
	// Tid attr
	*(pdata_start++) = 0x00;		// attr
	*(pdata_start++) = 0x03;		// key length
	*(pdata_start++) = 'T';
	*(pdata_start++) = 'i';
	*(pdata_start++) = 'd'; 		// key id
	*(pdata_start++) = 0x00;
	*(pdata_start++) = 0x04;		// value length

	*(pdata_start++) = *(pType++);
	*(pdata_start++) = *(pType++);
	*(pdata_start++) = *(pType++);
	*(pdata_start++) = *(pType++);	// value; default call

	(*pdata_end) = pdata_start;
}

void UploadPacket_Set_Position(uint8_t *pdata_start, Point_Struct *pPosition, uint8_t **pdata_end)
{
	// pl attr
	*(pdata_start++) = 0x10;		// attr
	*(pdata_start++) = 0x02;		// key length
	*(pdata_start++) = 'p';
	*(pdata_start++) = 'l'; 		// key id
	*(pdata_start++) = 0x00;
	*(pdata_start++) = 0x25;		// value length	 !!!

	// pl attr -- point-status
	*(pdata_start++) = 0x00;		// attr
	*(pdata_start++) = 0x01;		// key length
	*(pdata_start++) = 't'; 		// key id
	*(pdata_start++) = 0x00;
	*(pdata_start++) = 0x01;		// value length	 !!!
	*(pdata_start++) = pPosition->location_Status;

	// pl attr -- point-lat
	*(pdata_start++) = 0x00;		// attr
	*(pdata_start++) = 0x03;		// key length
	*(pdata_start++) = 'l'; 
	*(pdata_start++) = 'a'; 
	*(pdata_start++) = 't'; 		// key id
	*(pdata_start++) = 0x00;
	*(pdata_start++) = 0x04;		// value length	 !!!

	*(pdata_start++) = (uint8_t)((pPosition->Latitude)>>24);
	*(pdata_start++) = (uint8_t)((pPosition->Latitude)>>16);
	*(pdata_start++) = (uint8_t)((pPosition->Latitude)>>8);
	*(pdata_start++) = (uint8_t)(pPosition->Latitude);

	// pl attr -- point-lon
	*(pdata_start++) = 0x00;		// attr
	*(pdata_start++) = 0x03;		// key length
	*(pdata_start++) = 'l'; 
	*(pdata_start++) = 'o'; 
	*(pdata_start++) = 'n'; 		// key id
	*(pdata_start++) = 0x00;
	*(pdata_start++) = 0x04;		// value length	 !!!

	*(pdata_start++) = (uint8_t)((pPosition->Longitude)>>24);
	*(pdata_start++) = (uint8_t)((pPosition->Longitude)>>16);
	*(pdata_start++) = (uint8_t)((pPosition->Longitude)>>8);
	*(pdata_start++) = (uint8_t)(pPosition->Longitude);

	// pl attr -- point-alt
	*(pdata_start++) = 0x00;		// attr
	*(pdata_start++) = 0x03;		// key length
	*(pdata_start++) = 'a'; 
	*(pdata_start++) = 'l'; 
	*(pdata_start++) = 't'; 		// key id
	*(pdata_start++) = 0x00;
	*(pdata_start++) = 0x02;		// value length	 !!!

	*(pdata_start++) = (uint8_t)((pPosition->Alitude)>>8);
	*(pdata_start++) = (uint8_t)(pPosition->Alitude);

	(*pdata_end) = pdata_start;
}

void UploadPacket_Set_Speed(uint8_t *pdata_start, uint16_t speed, uint8_t **pdata_end)
{
	// s attr
	*(pdata_start++) = 0x00;		// attr
	*(pdata_start++) = 0x01;		// key length
	*(pdata_start++) = 's'; 		// key id
	*(pdata_start++) = 0x00;
	*(pdata_start++) = 0x02;		// value length

	*(pdata_start++) = (uint8_t)(speed>>8);
	*(pdata_start++) = (uint8_t)speed;		// value; default call

	(*pdata_end) = pdata_start;
}

void UploadPacket_Set_Direction(uint8_t *pdata_start, uint16_t direction, uint8_t **pdata_end)
{
	// d attr
	*(pdata_start++) = 0x00;		// attr
	*(pdata_start++) = 0x01;		// key length
	*(pdata_start++) = 'd'; 		// key id
	*(pdata_start++) = 0x00;
	*(pdata_start++) = 0x02;		// value length

	*(pdata_start++) = (uint8_t)(direction>>8);
	*(pdata_start++) = (uint8_t)direction;		// value; default call

	(*pdata_end) = pdata_start;
}

void UploadPacket_Set_Mileage(uint8_t *pdata_start, uint32_t mileage, uint8_t **pdata_end)
{
	// me attr
	*(pdata_start++) = 0x00;		// attr
	*(pdata_start++) = 0x02;		// key length
	*(pdata_start++) = 'm'; 
	*(pdata_start++) = 'e'; 		// key id
	*(pdata_start++) = 0x00;
	*(pdata_start++) = 0x04;		// value length

	*(pdata_start++) = (uint8_t)(mileage>>24);
	*(pdata_start++) = (uint8_t)(mileage>>16);
	*(pdata_start++) = (uint8_t)(mileage>>8);
	*(pdata_start++) = (uint8_t)(mileage);		// value; default call

	(*pdata_end) = pdata_start;
}


#ifdef	_EN_EX_AKV_
static void UploadPacket_Set_eX(uint8_t *pdata_start, Peripheral_info *info, uint8_t **pdata_end)
{
	// ex attr
	*(pdata_start++) = 0x00;		// attr
	*(pdata_start++) = 0x02;		// key length
	*(pdata_start++) = 'e'; 
	*(pdata_start++) = 'X'; 		// key id
	*(pdata_start++) = 0x00;
	*(pdata_start++) = 0x1A;		// value length

	// ex - lo
	*(pdata_start++) = 0x00;		// attr
	*(pdata_start++) = 0x02;		// key length
	*(pdata_start++) = 'l'; 
	*(pdata_start++) = 'o'; 		// key id
	*(pdata_start++) = 0x00;
	*(pdata_start++) = 0x02;		// value length

	*(pdata_start++) = (uint8_t)(info->LeftOli>>8);
	*(pdata_start++) = (uint8_t)(info->LeftOli);

	// ex - co
	*(pdata_start++) = 0x00;		// attr
	*(pdata_start++) = 0x02;		// key length
	*(pdata_start++) = 'c'; 
	*(pdata_start++) = 'o'; 		// key id
	*(pdata_start++) = 0x00;
	*(pdata_start++) = 0x02;		// value length

	*(pdata_start++) = (uint8_t)(info->CapacityOil>>8);
	*(pdata_start++) = (uint8_t)(info->CapacityOil);

	// ex - me
	*(pdata_start++) = 0x00;		// attr
	*(pdata_start++) = 0x02;		// key length
	*(pdata_start++) = 'm'; 
	*(pdata_start++) = 'e'; 		// key id
	*(pdata_start++) = 0x00;
	*(pdata_start++) = 0x04;		// value length

	*(pdata_start++) = (uint8_t)(info->mileage>>24);
	*(pdata_start++) = (uint8_t)(info->mileage>>16);
	*(pdata_start++) = (uint8_t)(info->mileage>>8);
	*(pdata_start++) = (uint8_t)(info->mileage);

	(*pdata_end) = pdata_start;
}
#endif

void UploadPacket_Set_TSMAP(uint8_t *pdata_start, Device_info *info, uint8_t **pdata_end)
{
	// TSMAP attr
	*(pdata_start++) = 0x00;		// attr
	*(pdata_start++) = 0x05;		// key length
	*(pdata_start++) = 'T'; 
	*(pdata_start++) = 'S'; 
	*(pdata_start++) = 'M'; 
	*(pdata_start++) = 'A'; 
	*(pdata_start++) = 'P'; 		// key id
	*(pdata_start++) = 0x00;
	*(pdata_start++) = 0x12;		// value length

	// TSMAP - t
	*(pdata_start++) = 0x00;		// attr
	*(pdata_start++) = 0x01;		// key length
	*(pdata_start++) = 't'; 		// key id
	*(pdata_start++) = 0x00;
	*(pdata_start++) = 0x01;		// value length
	*(pdata_start++) = info->car_type;

	// TSMAP - a
	*(pdata_start++) = 0x00;		// attr
	*(pdata_start++) = 0x01;		// key length
	*(pdata_start++) = 'a'; 		// key id
	*(pdata_start++) = 0x00;
	*(pdata_start++) = 0x01;		// value length
	*(pdata_start++) = info->car_run;

	// TSMAP - g
	*(pdata_start++) = 0x00;		// attr
	*(pdata_start++) = 0x01;		// key length
	*(pdata_start++) = 'g'; 		// key id
	*(pdata_start++) = 0x00;
	*(pdata_start++) = 0x01;		// value length
	*(pdata_start++) = info->car_wheel;

	(*pdata_end) = pdata_start;
}
