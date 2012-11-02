/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-10-18 09:38
#      Filename : UDP_Lib.h
#   Description : not thing...
#
=============================================================================*/

#ifndef _UDP_LIB_H_
#define _UDP_LIB_H_

#include <stdint.h>
#include <stdio.h>
#include "GSM-error.h"

int 	  ascii_2_hex		(uint8_t *pAscii, uint8_t *pHex, int len);
int 	  hex_2_ascii		(char *pHex, char *pAscii, int len);
GSM_Error UDP_SendPacket	(char *pBuff, int len);
GSM_Error UDP_ReceivePacket	(int *pLink_num, int *pData_index, int *pData_len, char *pBuff);


#endif



