/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-10-18 09:59
#      Filename : PositionUpload.h
#   Description : not thing...
#
=============================================================================*/

#ifndef		_POSITION_UPLOAD_H_
#define		_POSITION_UPLOAD_H_

#include "GSM-error.h"
#include "TLP.h"
#include "AppLayer.h"
#include "ReportCondition.h"
#include "BtypeTransmit.h"

//#define		_EN_EX_AKV_

typedef struct _Peripheral_
{
	uint16_t		LeftOli;
	uint16_t		CapacityOil;
	uint32_t		mileage;
}Peripheral_info;

typedef struct _DeviceStatus_
{
	uint8_t			car_type;	// 0x00 -> empty car; 0x01 -> heavy car; 0x02 -> task car
	uint8_t			car_run;	// 0x00 -> not running; 0x01 -> in running
	uint8_t			car_wheel;	// 0x00 ; 0x01; 0x02
}Device_info;

typedef struct _PostionUpload_info_
{
	uint8_t				need_return;
	uint8_t				*pType;
	Point_Struct 		position;
	uint16_t			speed;			// Km/h
	uint16_t			direction;		// 0x00 ~ 0x167(359)
	uint32_t			mileage;		// Km
	Peripheral_info		Oil_info;
	Device_info			car_info;
}PostionUpload_info;


void UploadPacket_Set_Position	 (uint8_t *pdata_start, Point_Struct *pPosition, uint8_t **pdata_end);
void UploadPacket_Set_Mileage 	 (uint8_t *pdata_start, uint32_t mileage, uint8_t **pdata_end);
void UploadPacket_Set_Direction	 (uint8_t *pdata_start, uint16_t direction, uint8_t **pdata_end);
void UploadPacket_Set_Speed		 (uint8_t *pdata_start, uint16_t speed, uint8_t **pdata_end);
void UploadPacket_Set_TSMAP		 (uint8_t *pdata_start, Device_info *info, uint8_t **pdata_end);
void UploadPacket_Set_NeedReturn (uint8_t *pdata_start, uint8_t flag, uint8_t **pdata_end);

APP_Error PositionUpload		 (PostionUpload_info *pInfo, Rule_Effective *pTry_info);

#endif
