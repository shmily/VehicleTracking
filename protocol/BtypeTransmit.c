
#include <stdint.h>
#include <math.h>
#include "UDP_Lib.h"
#include "GSM_Hal.h"
#include "GSM-error.h"

#include <time.h>
#include <errno.h>

#define	_Delay_		200

static uint8_t Btype_TxBuff[2048];
static uint8_t Btype_RxBuff[2048];

static GSM_Error ReceivePacket(int *pLink_num, int *pData_index, int *pData_len, char *pBuff);
static GSM_Error GetPaseAckPacket(char *pError_code);


static int msleep (unsigned long milisec)
{
     struct timespec req = {0};
     time_t sec = (int)(milisec / 1000);
     milisec = milisec - (sec * 1000);
     req.tv_sec = sec;            			
     req.tv_nsec = milisec * 1000000L;
     while (nanosleep (&req, &req) == -1 && errno == EINTR) {
         continue;
     }
     return (1);
}


// interval : ms
GSM_Error Btype_SendPacket(char *pBuff, int len, int retry, int interval, char *pError_code)
{
	GSM_Error 	res = ERR_NONE;
	GSM_Error 	ack_res;
	int 		times;
	int 		interval_cnt;

	char		Unread_sum;
	char		Sum;
	char		error_code;
	char		ACK_flg;
	
	hex_2_ascii(pBuff, (char *)Btype_TxBuff, len);

	interval_cnt = interval/_Delay_;

	for(times=0; times<retry; times++){							// we try 

		res = GSM_SendPacket((char *)Btype_TxBuff);

		if(res == ERR_NONE){									// wait for the ACK

			ack_res = ERR_UNKNOWN;
			ACK_flg = 0;
			do{
				msleep(_Delay_);

				res = GSM_GetPacketInfo(&Unread_sum, &Sum);
				if(res == ERR_NONE){
					if(Unread_sum>0){							// we receive something
						ack_res = GetPaseAckPacket(&error_code);

						if(ack_res != ERR_UNKNOWN){				// we receive the ack
							ACK_flg = 1;
							break;		
						}					
					}
				}

				interval_cnt--;
			}while(interval_cnt>0);

			if(ACK_flg == 1) {									// if we get ack, return
				(*pError_code) = error_code;
				break;
			}
		} else {
			return ERR_DEVICENOTWORK;							// if we faild, return
		}
	}

	return ack_res;
}


static GSM_Error GetPaseAckPacket(char *pError_code)
{
	GSM_Error 	res = ERR_NONE;
	int 		Link_num;
	int 		Data_index;
	int 		Data_len;
	uint8_t		rx_load[512];
	uint8_t		error_code;

	res = ReceivePacket(&Link_num, &Data_index, &Data_len, (char *)rx_load);
	error_code = rx_load[24];

	if(res == ERR_NONE){
		if(Data_len==25){				// the ack packet is 25 bytes
			if(error_code == 0x00){
				res = ERR_NONE;
			} else {
				fprintf(stderr,"%s : Error code = 0x%02X\n",__func__,error_code);
				res = ERR_NOTCONNECTED;
			}
		} else {
			res = ERR_UNKNOWN;
		}
	} else {
		res = ERR_UNKNOWN;
	}

	*pError_code = error_code;
	return res;
}


static GSM_Error ReceivePacket(int *pLink_num, int *pData_index, int *pData_len, char *pBuff)
{
	GSM_Error res = ERR_NONE;
	int       length;
	
	length = 0;
	res = GSM_ReceivePacket(pLink_num, pData_index, &length, (char *)Btype_RxBuff);

	
	if(res==ERR_NONE){
		*pData_len = length;
		ascii_2_hex(Btype_RxBuff,(uint8_t *) pBuff, 2*length);	
	}
	
	return res;
}
