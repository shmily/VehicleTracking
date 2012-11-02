/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-10-18 09:37
#      Filename : UDP_Lib.c
#   Description : not thing...
#
=============================================================================*/

#include <stdint.h>
#include "UDP_Lib.h"
#include "GSM_Hal.h"
#include "GSM-error.h"

static uint8_t UDP_TxBuff[2048];
static uint8_t UDP_RxBuff[2048];

int ascii_2_hex(uint8_t *pAscii, uint8_t *pHex, int len)
{
	int i,j,tmp_len;
	uint8_t tmpData;
	uint8_t *O_buf = pAscii;
	uint8_t *N_buf = pHex;
	
	for(i = 0; i < len; i++)
	{
		if ((O_buf[i] >= '0') && (O_buf[i] <= '9')){
			tmpData = O_buf[i] - '0';
		}
		else if ((O_buf[i] >= 'A') && (O_buf[i] <= 'F')){ 	//A....F
			tmpData = O_buf[i] - 0x37;
		}
		else if ((O_buf[i] >= 'a') && (O_buf[i] <= 'f')){ 	//a....f
			tmpData = O_buf[i] - 0x57;
		}
		else{
			return -1;
		}
		
		O_buf[i] = tmpData;
	}
	
	for(tmp_len = 0,j = 0; j < i; j+=2){
		N_buf[tmp_len++] = (O_buf[j]<<4) | O_buf[j+1];
	}
	
	return tmp_len;
}


int hex_2_ascii(char *pHex, char *pAscii, int len)
{
	const char ascTable[17] = {"0123456789ABCDEF"};
	char  *tmp_p = pAscii;
	int   i, pos;
	
	pos = 0;
	for(i = 0; i < len; i++){
		tmp_p[pos++] = ascTable[pHex[i] >> 4];
		tmp_p[pos++] = ascTable[pHex[i] & 0x0f];
	}
	
	tmp_p[pos] = '\0';
	
	return pos;
}


GSM_Error UDP_SendPacket(char *pBuff, int len)
{
	GSM_Error res;
	
	hex_2_ascii(pBuff, (char *)UDP_TxBuff, len);
	
	res = GSM_SendPacket((char *)UDP_TxBuff);
	
	return res;
}

GSM_Error UDP_ReceivePacket(int *pLink_num, int *pData_index, int *pData_len, char *pBuff)
{
	GSM_Error res;
	int       length;
	
	length = 0;
	*pData_len = 0;
	
	res = GSM_ReceivePacket(pLink_num, pData_index, &length, (char *)UDP_RxBuff);

	
	if(res==ERR_NONE){
		*pData_len = length;
		ascii_2_hex(UDP_RxBuff,(uint8_t *) pBuff, 2*length);	
	}
	
	return res;
}


