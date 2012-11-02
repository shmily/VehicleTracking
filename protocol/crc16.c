/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-10-18 13:53
#      Filename : crc16.c
#   Description : not thing...
#
=============================================================================*/

#include <stdint.h>
#include "crc16.h"


short Caculate(const void *pdata,  int length)
{
	short crc = (short) 0xFFFF; 
	int i, j;
	char *pByte;
	char c15, bit;

	pByte = (char *)pdata;

	for (i = 0; i < length; i++) {
		for (j = 0; j < 8; j++) {
			c15 = ((crc >> 15 & 1) == 1);
			bit = (((*pByte) >> (7 - j) & 1) == 1);
			crc <<= 1;
			if (c15 ^ bit) crc ^= 0x1021; 
		}
		pByte++;
	}
	return crc;
}
