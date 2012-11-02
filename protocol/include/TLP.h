/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-10-18 10:02
#      Filename : TLP.h
#   Description : not thing...
#
=============================================================================*/

#ifndef __TLP_H__
#define __TLP_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define 	__DEBUG__ 		1

#ifdef __DEBUG__
	#define DEBUG(fmt,args...) fprintf(stderr,fmt, ## args)
#else
	#define DEBUG(fmt,args...)
#endif

#define HTONS(n) ((((uint16_t)((n) & 0xff)) << 8) | (((n) & 0xff00) >> 8))

#define HTONL(n) ((((uint32_t)(n) & 0xff000000) >> 24) |\
                  (((uint32_t)(n) & 0x00ff0000) >> 8)  |\
                  (((uint32_t)(n) & 0x0000ff00) << 8)  |\
                  (((uint32_t)(n) & 0x000000ff) << 24))


// Transmit Error defind
typedef enum{
	
	// transmit ok
	ERROR_NONE = 0,

	// CRC error
	ERROR_CRC,
	
	// unknow error
	ERROR_UNKNOW
}TLP_Error;


// Transmit type def

#define		TypeA	0x00
#define		TypeB	0x10
#define		TypeC	0x20
#define		TypeD	0x30
#define		TypeE	0x40
#define		TypeX	0xFF

// input packet
typedef struct _InputPacket_
{
	uint32_t length;
	uint8_t  Data[1024];
}Packet_Struct;


// transport layer head
typedef struct _TLP_Head_
{
	uint16_t	crc16;
	uint8_t		version;		// 0x00
	uint8_t		length;
	
	uint8_t		type;			
	uint8_t		medium;			// UDP : 0x03
	uint8_t		encrypt;		// none : 0x00
	uint8_t		reserve;		// 0x00
	
	uint32_t	SEQ_num;
	uint32_t	DEV_IDH;
	uint32_t	DEV_IDL;
	uint32_t	timestamp;
}TLP_Head_Struct;

// Input Pack struct
typedef struct _InputPack_
{
	uint8_t  TransmitType;
	uint32_t SEQ_num;
	uint8_t  *DatPointer;
	uint16_t length;
}PacketInfo;


// function

TLP_Error TLP_PacketDevide(Packet_Struct *TLP_Pack, PacketInfo *APP_Packet, int *packet_type);

#endif
