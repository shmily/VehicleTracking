/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-10-16 10:48
#      Filename : ImageTransmit.c
#   Description : not thing...
#
=============================================================================*/

#include "crc16.h"
#include "uart.h"
#include "AtTransmit.h"
#include "GSM-error.h"
#include "GSM_Hal.h"
#include "UDP_Lib.h"

#include "ImageTransmit.h"
#include "md5.h"
#include "InputHandle.h"
#include "DtypeTransmit.h"

#include <string.h>
#include <stdio.h>
#include <unistd.h>   // for pause()  
#include <signal.h>   // for signal()  
#include <sys/time.h> // struct itimeral. setitimer()  



int _Debug_Close = 0;

DT_StateFlag 		DT_State;

static char			ImagePacket[_DT_SIZE];
static Image_Struct Image_Info;

static DT_Packet	Packet_Info;
extern Retry_Info	RetryPacket;

static int 		 GetImage_Info(const char *path, const char *name);
static APP_Error ImagePacketTransmit(const void *pdata, int flag);
static int 		 GetImagePacket(Image_Struct *pImage, int index, char *pBuff);
//static int 		 LowLevel_GetPacket(void);


DT_StateFlag Get_DT_State(void)
{
	return DT_State;
}

void Set_DT_State(DT_StateFlag state)
{
	DT_State = 	state;
}

// this is the image transmit process
int ImageTransmit_init(const char *path, const char *name)
{
	int		res;		
	
	res = GetImage_Info(path, name);
	
	if(res == DT_File_OK){
		res = DT_UpdateRequest(&Image_Info);
		if(res == APP_ERR_NONE){
		
			Packet_Info.lastPacket = 0;
			Packet_Info.num = 0;
			Packet_Info.next_num = 0;
			RetryPacket.error_code = 0x01;
			RetryPacket.packet_num = 0;
			memset(RetryPacket.packetIndex, 0, sizeof(int)*16);
			
			return DT_ConnectREQ_SEND;
		} else {
			return DT_SendError;
		}
	}

	return DT_SendError;
}

int ImageTransmit_loop(int flag)
{
/*
	if((DT_Image_Over==DT_State)||(DT_SendError==DT_State)){	
		fprintf(stderr, "%s : > Image send over, close the connection...\n", __func__);
		DT_DisConnectRequest(0x00);
		sleep(1);
	}*/

	switch(DT_State){

		case DT_Idle :
			break;

		case DT_ConnectREQ_ERR :
			fprintf(stderr, "%s : > Server connect error...\n", __func__);
			break;

		case DT_ConnectREQ_OK :
		case DT_Packet_ACK :
			fprintf(stderr, "%s : > Start to send packet...\n", __func__);
			ImagePacketTransmit(NULL,0);
			break;

		case DT_DisconnectREQ_OK :
			fprintf(stderr, "%s : > Close connection OK...\n", __func__);
			DT_State = DT_Idle;
			break;

		case DT_DisconnectREQ_ERR :
			fprintf(stderr, "%s : > Close connection ERR...\n", __func__);
			break;

	//	case DT_Image_Over:
	//	   	fprintf(stderr, "%s : > the DT_State = DT_Image_Over...\n", __func__);
	//		DT_State = DT_Idle;
	// 		break;	   

		default :
			fprintf(stderr, "%s : > Image transmit state ERR...\n", __func__);
			DT_State = DT_Idle;
			break;
	}
	
	if((DT_Image_Over==DT_State)||(DT_SendError==DT_State)){	
		fprintf(stderr, "%s : > Image send over, close the connection...\n", __func__);
		DT_DisConnectRequest(0x00);
		DT_State = DT_Idle;
		sleep(1);
	}
	
	return 0;
}


static int GetImage_Info(const char *path, const char *name)
{	
	FILE *pFile;
	
	// for md5
	md5_state_t state;
	int			i;
	int			len;
	char	    buff[1024];
	
	pFile = fopen(path, "r");
	Image_Info.pFile = pFile;
	
	if(pFile == NULL){			// open file faild
		fprintf(stderr, "%s : Opend file \"%s\" faild...\n", __func__, path);
		return DT_File_ERR;
	}
	
	// get file szie
	fseek(pFile, SEEK_SET, SEEK_END);
	Image_Info.size = ftell(pFile);
	fseek(pFile, 0, SEEK_SET);
	
	Image_Info.Total_Packet = (Image_Info.size + (_DT_SIZE-1))/_DT_SIZE;
	
	// get file md5
	md5_init(&state);
	
	do{
		len = fread(buff, 1, 1024, pFile);
		md5_append(&state, (const md5_byte_t *)buff, len);
	}while(len==1024);
	
	md5_finish(&state, (md5_byte_t *)Image_Info.MD5);
	
	fprintf(stderr, "MD5 of \"%s\" : ", path);
	for(i=0;i<16;i++){
		fprintf(stderr, "%02X", Image_Info.MD5[i]);
	}	
	fprintf(stderr, "\n");
	
	fseek(pFile, 0, SEEK_SET);

	// type
	Image_Info.type = 0x07;
	
	// file name need to be add **
	Image_Info.name[0] = *(name++);
	Image_Info.name[1] = *(name++);;
	Image_Info.name[2] = *(name++);;
	Image_Info.name[3] = *(name++);;
	Image_Info.name[4] = *(name++);;
	Image_Info.name[5] = *(name++);;


	fprintf(stderr, "%s : image size   : %d Byte\n", __func__, Image_Info.size);
	fprintf(stderr, "%s : total Packet : %d\n", __func__, Image_Info.Total_Packet);
	
	return DT_File_OK;
}



static APP_Error ImagePacketTransmit(const void *pdata, int flag)
{
	APP_Error res;
	int i = 0;
	int	ext_num = 0;
	int	needSend_num = 0;
	
	fprintf(stderr, "%s : > Start to send Image Packet...!\n", __func__);
	
	ext_num = RetryPacket.packet_num;
	

		// get the next transmit index
		if(((RetryPacket.error_code)==0x01)||((RetryPacket.error_code)==0x02)){	
		
			if(Packet_Info.lastPacket!=1){													// if we did not reach the end of file, we need to get new packet index.
				for(i=0; i<(16 - ext_num); i++){
					RetryPacket.packetIndex[ext_num + i] = Packet_Info.next_num + i;
				}
				Packet_Info.next_num = Packet_Info.next_num + i;
				
				if(Image_Info.Total_Packet <= Packet_Info.next_num){
					fprintf(stderr, ">>>> last packet ...\n");
					Packet_Info.lastPacket = 1;
					needSend_num = 16 - (Packet_Info.next_num - Image_Info.Total_Packet);	// the last packet num
				} else {
					needSend_num = 16;
				}
			}else{																			// if we had reach the end of file, we do not to get new packet index.
				if(RetryPacket.error_code == 0x01){
					fprintf(stderr, ">>>> Image send over...\n");
					DT_State = DT_Image_Over; 	
					return APP_ERR_NONE;
				} else {
					needSend_num = ext_num;
				}
			}
		} else if(RetryPacket.error_code==0x03){
			fprintf(stderr, ">>>> sever request to resend the image...\n");
			DT_State = DT_SendError;	
			_Debug_Close = 1;
			return APP_ERR_NONE;
		}

		fprintf(stderr, "%s : > needSend_num = %d\n", __func__, needSend_num);
		fprintf(stderr, "%s : > Packet index : ", __func__);
		for(i=0; i<needSend_num; i++){
			fprintf(stderr, "[%d] ", RetryPacket.packetIndex[i]);
		}

		// send packet
		fprintf(stderr, "\n%s : > Send %d packets...\n", __func__, needSend_num);
		for(i=0; i<needSend_num; i++){
			Packet_Info.actual_len 	= GetImagePacket(&Image_Info, RetryPacket.packetIndex[i], ImagePacket);
			Packet_Info.need_ack 	= 0x00;
			Packet_Info.num      	= RetryPacket.packetIndex[i];
			Packet_Info.pdata    	= ImagePacket;
			
			
			fprintf(stderr, "actual_len = %d\n", Packet_Info.actual_len);	
			fprintf(stderr, "num        = %d\n", Packet_Info.num);
			fprintf(stderr, "Address    = %d\n", (int)Packet_Info.pdata);
			
			res = DT_UpdatePacket(&Image_Info, &Packet_Info);
			
			if(res!=APP_ERR_NONE){	
				DT_State = DT_SendError;
				return APP_ERR_SEND;
			}
		    //LowLevel_GetPacket();	
		}

		usleep(100000);
		
		DT_RSendRequest(&Image_Info, RetryPacket.packetIndex[needSend_num]);
		
		return APP_ERR_NONE;
}



static	int GetImagePacket(Image_Struct *pImage, int index, char *pBuff)
{
	FILE *pFile;
	int	 len;
				
	pFile = pImage->pFile;
	fseek(pFile, (index*_DT_SIZE), SEEK_SET);
	len = fread(pBuff, 1, _DT_SIZE, pFile);
				
	return len;
}


/*
static int LowLevel_GetPacket(void)
{
	char unread_sum;
	char packet_sum;
	char buff[128];

	int  CRC_Receive;
	int  CRC_Caculate;
	int	 i;	
	int	 link_num;
	int	 data_index;
	int	 data_len = 0;
	
	GSM_GetPacketInfo(&unread_sum, &packet_sum);
	
	if(unread_sum>0){
		memset(buff,'\0',128);
		UDP_ReceivePacket(&link_num, &data_index, &data_len, buff);
		
		DEBUG("Receive Packet ...\n");
		DEBUG("link num    = %d\n",link_num);
		DEBUG("data index  = %d\n",data_index);
		DEBUG("data length = %d\n",data_len);
		
		CRC_Receive  = (buff[0]<<8) + buff[1];
		CRC_Caculate = Caculate(&buff[2],data_len-2)&0xFFFF;				
		fprintf(stderr, "CRC = 0x%04X\n",CRC_Caculate);
		if(CRC_Receive == CRC_Caculate){
			fprintf(stderr,"CRC is OK\n");
		}else{
			fprintf(stderr,"CRC is ERROR\n");
		}
	
		fprintf(stderr, "\n > ^^^^^^^^^^^^^^^^^^^^^^^^\n");

		for(i=0; i<data_len; i++){
			fprintf(stderr, "0x%02X ", buff[i]);
		}	
		fprintf(stderr, "\n < VVVVVVVVVVVVVVVVVVVVVVVVV\n");
	}
	
	return data_len;
}*/
