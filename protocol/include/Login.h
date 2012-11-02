/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-10-18 09:49
#      Filename : Login.h
#   Description : not thing...
#
=============================================================================*/

#ifndef _LOGIN_H_
#define	_LOGIN_H_

#include <stdio.h>

#include "GSM_Hal.h"
#include "GSM-error.h"
#include "TLP.h"
#include "AppLayer.h"
#include "UDP_Lib.h"
#include "crc16.h"
#include "Timestamp.h"
#include "SequenceNumber.h"

GSM_Error Send_LoginPacket		(void);
APP_Error Wait_LoginACK			(void *pdata);
GSM_Error Send_RegisterPacket	(void);
APP_Error Wait_RegisterACK		(void *pdata);
#endif
