/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-10-18 09:52
#      Filename : ObjectRequest.h
#   Description : not thing...
#
=============================================================================*/

#ifndef _OBJ_REQ_H_
#define _OBJ_REQ_H_

#include "AppLayer.h"

// action code
#define		_HeartBeat		0x0003

APP_Error ObjectRequestHandle	(CommandType *command);
APP_Error HeartBeat_Request		(TLP_Head_Struct *pHeadInfo);

#endif
