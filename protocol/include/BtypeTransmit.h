/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-10-18 09:48
#      Filename : BtypeTransmit.h
#   Description : not thing...
#
=============================================================================*/

#ifndef _BTYPE_TRANSMIT_H_
#define _BTYPE_TRANSMIT_H_

#include <stdio.h>
#include "GSM-error.h"

// error code defind

typedef enum {
	TX_SUCCESS = 0x00,
	TX_CTRLERR = 0x01,
	TX_EMPTY   = 0x02,
	TX_IDERR   = 0x03,
	TX_GWERR   = 0x04,
}TX_Error;

GSM_Error Btype_SendPacket(char *pBuff, int len, int retry, int interval, char *pError_code);

#endif



