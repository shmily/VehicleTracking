/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-10-18 14:17
#      Filename : Btype_InputHandle.h
#   Description : not thing...
#
=============================================================================*/

#ifndef __BTYPE_INPUT_HANDLE_
#define __BTYPE_INPUT_HANDLE_

#include <stdio.h>

#define 	_Btype_REQ_ACK		0x10
#define		_Btype_Packet_ACK	0x11


void Btype_InputHandle(const void *pdata, int length);

#endif

