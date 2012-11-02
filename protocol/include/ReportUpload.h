/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-10-18 10:01
#      Filename : ReportUpload.h
#   Description : not thing...
#
=============================================================================*/

#ifndef		_REPORT_UPLOAD_H
#define		_REPORT_UPLOAD_H

#include "GSM-error.h"
#include "TLP.h"
#include "AppLayer.h"
#include "ReportCondition.h"
#include "BtypeTransmit.h"
#include "PositionUpload.h"


typedef struct _ReportUpload_info_
{
	uint8_t				need_return;
	Point_Struct 		position;
	uint16_t			speed;			// Km/h
	uint16_t			direction;		// 0x00 ~ 0x167(359)
	uint32_t			mileage;		// Km
	Peripheral_info		Oil_info;
	Device_info			car_info;
}ReportUpload_info;



#endif

