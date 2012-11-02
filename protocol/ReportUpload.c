
#include "crc16.h"
#include "ReportUpload.h"
#include <string.h>

#define		_R_UpLoadPacketLen_		512	
uint8_t R_Upload_Data[_R_UpLoadPacketLen_];

static void UploadPacket_Set_Priority(uint8_t *pdata_start, uint8_t priority, uint8_t **pdata_end);
static void UploadPacket_Set_Rid(uint8_t *pdata_start, uint8_t *pRid, int id_len, uint8_t **pdata_end);
static void UploadPacket_Set_Rname(uint8_t *pdata_start, uint16_t rname, uint8_t **pdata_end);
static void En_RU_Packet(uint8_t *pdata_start, TLP_Head_Struct *pHeader, Report_Info *pReport_info, ReportUpload_info *pRU_info, int *plength);

APP_Error ReportUpload(ReportUpload_info *pInfo, Report_Info *pTry_info)
{
	GSM_Error		res;
	TLP_Head_Struct head;
	int				length;
	char			error_code;

	head.SEQ_num = 0x00000000;
	En_RU_Packet(R_Upload_Data, &head, pTry_info, pInfo, &length);

	res = Btype_SendPacket((char *)R_Upload_Data, length, pTry_info->Cnt, pTry_info->Interval*1000ul, &error_code);

	if(res!=ERR_NONE){
		DEBUG("%s : PositionUpload ...Error!\n",__func__);
		DEBUG("ERROR Code : 0x%02X\n", error_code);
		return APP_ERR_SEND;
	} else {
		DEBUG("%s : PositionUpload ...OK\n",__func__);
		return APP_ERR_NONE;
	}
}

static void En_RU_Packet(uint8_t *pdata_start, TLP_Head_Struct *pHeader, Report_Info *pReport_info, ReportUpload_info *pRU_info, int *plength)
{
	uint16_t		crc_resault;
	int				len;
	TLP_Head_Struct	*PacketHead;
	uint8_t			*pData;
	uint8_t			*pdata_end;

	memset(R_Upload_Data, 0, _R_UpLoadPacketLen_);		// clean it

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
	*(pData++) = 0x03;								// report upload

	// add need return
	UploadPacket_Set_NeedReturn(pData, pRU_info->need_return, &pdata_end);

	// set rname
	pData = pdata_end;
	UploadPacket_Set_Rname(pData, pReport_info->action, &pdata_end);
	
	// set rid
	pData = pdata_end;
	UploadPacket_Set_Rid(pData, pReport_info->RID, pReport_info->R_len, &pdata_end);

	// set priority
	pData = pdata_end;
	UploadPacket_Set_Priority(pData, pReport_info->Priority, &pdata_end);

	// set position
	pData = pdata_end;
	UploadPacket_Set_Position(pData, &(pRU_info->position), &pdata_end);

	// add speed
	pData = pdata_end;
	UploadPacket_Set_Speed(pData, pRU_info->speed, &pdata_end);

	// add direction
	pData = pdata_end;
	UploadPacket_Set_Direction(pData, pRU_info->direction, &pdata_end);

	// add mileage
	pData = pdata_end;
	UploadPacket_Set_Mileage(pData, pRU_info->mileage, &pdata_end);	

	// add TSMAP AKV
	pData = pdata_end;
	UploadPacket_Set_TSMAP(pData, &(pRU_info->car_info), &pdata_end);	

	// get data length 
	pData = pdata_end;
	len = pData - pdata_start;

	// get the crc16
	pData         = pdata_start + 2;	
	crc_resault   = Caculate(pData, (len-2));
	PacketHead->crc16 = HTONS(crc_resault);

	(*plength) = len;
}




static void UploadPacket_Set_Rname(uint8_t *pdata_start, uint16_t rname, uint8_t **pdata_end)
{
	// Tid attr
	*(pdata_start++) = 0x00;		// attr
	*(pdata_start++) = 0x05;		// key length
	*(pdata_start++) = 'r';
	*(pdata_start++) = 'n';
	*(pdata_start++) = 'a';
	*(pdata_start++) = 'm';	
	*(pdata_start++) = 'e'; 		// key id
	*(pdata_start++) = 0x00;
	*(pdata_start++) = 0x02;		// value length

	*(pdata_start++) = (uint8_t)(rname>>8);
	*(pdata_start++) = (uint8_t)(rname);	// value; default call

	(*pdata_end) = pdata_start;
}


static void UploadPacket_Set_Rid(uint8_t *pdata_start, uint8_t *pRid, int id_len, uint8_t **pdata_end)
{
	// Tid attr
	*(pdata_start++) = 0x00;		// attr
	*(pdata_start++) = 0x03;		// key length
	*(pdata_start++) = 'r';
	*(pdata_start++) = 'i';	
	*(pdata_start++) = 'd'; 		// key id
	*(pdata_start++) = (uint8_t)(id_len>>8);
	*(pdata_start++) = (uint8_t)(id_len);;		// value length

	memcpy(pdata_start, pRid, id_len);

	(*pdata_end) = pdata_start + id_len;
}

static void UploadPacket_Set_Priority(uint8_t *pdata_start, uint8_t priority, uint8_t **pdata_end)
{
	// Tid attr
	*(pdata_start++) = 0x00;		// attr
	*(pdata_start++) = 0x01;		// key length
	*(pdata_start++) = 'p'; 		// key id
	*(pdata_start++) = 0x00;
	*(pdata_start++) = 0x01;		// value length

	*(pdata_start++) = priority;	// value; default call

	(*pdata_end) = pdata_start;
}

//
//  if this is need for ack , please add
//
#ifdef _EN_RMAP_
static void UploadPacket_Set_Rmap(uint8_t *pdata_start, void *pValue, uint8_t **pdata_end)
{
	(*pdata_end) = pdata_start;
}
#endif
