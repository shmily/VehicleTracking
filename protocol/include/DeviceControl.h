/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-10-18 09:49
#      Filename : DeviceControl.h
#   Description : not thing...
#
=============================================================================*/

#ifndef _DEVICE_CONTROL_H_
#define _DEVICE_CONTROL_H_

#include "AppLayer.h"


// antion code 
#define ACTION_ROLLCALL			0x0002
#define ACTION_ROLLCALL_ACK		0x0002


APP_Error DeviceControlHandle(CommandType *command);

#endif
