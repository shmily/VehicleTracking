/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-10-18 09:48
#      Filename : AppLayer.h
#   Description : not thing...
#
=============================================================================*/

#ifndef _APP_LAYER_
#define _APP_LAYER_

#include "TLP.h"

// App Error code 
typedef enum{
	
	// OK
	APP_ERR_NONE = 0,
	
	// app error send
	APP_ERR_SEND,
	
	// app error unknow
	APP_ERR_UNKNOW,
}APP_Error;


// packet head info
#define		_VERSION		0x00
#define 	_TransmitCtrl	0x80
#define 	_TransmitType	0x03
#define		_Encrypt		0x00
#define     _Reserve		0x00
#define		_DEV_IDH		0xFFFFF151
#define		_DEV_IDL		0x40539442


// control type
#define		DEV_CONTROL		0x01
#define		PER_CONTROL		0x02
#define		PER_SENDDAT		0x03
#define		PER_RECEDAT		0x04

#define		RULE_SET		0x0D
#define		RULE_DEL		0x0E
#define		RULE_CHECK		0x0F
#define		RULE_SWITCH		0x10

#define		REPORT_SET		0x11
#define		REPORT_DEL		0x12
#define		REPORT_CHECK	0x13
#define		REPORT_SWITCH	0x14

#define		UPLOAD_PACKET	0x40
#define		HEART_BEAT		0x41

// App packet head
typedef struct _CommandType_
{
	uint8_t  	Type;
	uint16_t 	Action;
	uint8_t	 	*pData;
	uint16_t 	length;
	PacketInfo 	*plowLevelInfo;
}CommandType;

void InputCommandHandle(PacketInfo *APP_Packet);

#endif
