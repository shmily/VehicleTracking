/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-10-18 10:06
#      Filename : DtypeTransmit.h
#   Description : not thing...
#
=============================================================================*/

#ifndef _BTYPE_TRANSMIT_H_
#define _BTYPE_TRANSMIT_H_

#include <stdio.h>
#include "AppLayer.h"
#include "InputHandle.h"

#define		_DT_SIZE			512

// Control typde define
#define		_DT_UPDATE			0xB0
#define		_DT_UPDATE_ACK		0x30
#define		_DT_PACKET			0xB1
#define		_DT_PACKET_ACK		0x31
#define		_DT_RSEND			0xB2
#define		_DT_RSEND_ACK		0x32
#define		_DT_RCONNECT		0xB3
#define		_DT_RCONNECT_ACK	0x33
#define		_DT_DISCONNECT		0xB4
#define		_DT_DISCONNECT_ACK	0x34

typedef struct _Image
{
	FILE 	*pFile;
	int		size;
	int		Total_Packet;
	char	MD5[16];
	char	name[8];
	char	type;
}Image_Struct;

typedef struct _DT_Packet
{
	char	need_ack;
	int		num;
	int		next_num;
	int		lastPacket;
	int     actual_len;
	void	*pdata;
}DT_Packet;

APP_Error DT_UpdateRequest		(Image_Struct *pImage);
APP_Error DT_UpdatePacket		(Image_Struct *pImage, DT_Packet *pPacket);
APP_Error DT_RSendRequest		(Image_Struct *pImage, int packet_num);
APP_Error DT_DisConnectRequest	(char error_code);

#endif



