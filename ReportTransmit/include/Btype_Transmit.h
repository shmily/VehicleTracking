/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-10-18 14:20
#      Filename : Btype_Transmit.h
#   Description : not thing...
#
=============================================================================*/

#ifndef __BTYPE_TRANSMIT_H_
#define __BTYPE_TRANSMIT_H_

#include <stdio.h>

// Control typde define
#define		_Btype_REQ 			((1<<7)|0x10)
#define		_Btype_REQ_ACK		((0<<7)|0x10)
#define		_Btype_Packet 		((1<<7)|0x11)
#define		_Btype_Packet_ACK	((0<<7)|0x11)

APP_Error Btype_SendRequest(const void *pdata_src, int length);
APP_Error Btype_SendPacket (const void *pdata_src, int length);

#endif
