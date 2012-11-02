/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-10-26 10:44
#      Filename : ExtendFounction.c
#   Description : not thing...
#
=============================================================================*/

#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/file.h>
#include <sys/time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>

#include "uart.h"
#include "AtTransmit.h"
#include "GSM-error.h"
#include "GSM_Hal.h"
#include "UDP_Lib.h"
#include "Login.h"



//
// this will not return, untill we login success...
//
void Login_Process(char *buff)
{
	GSM_Error 		err;
	char			unread_sum;
	char			packet_sum;

	int 			link_num;
	int				data_index;
	int 			data_len;
	int				CRC_Caculate;
	int				CRC_Receive;

	int				i;
	int				is_LogIn = 0;

	// Login process
	while(1){

		fprintf(stderr,"%s","\033[1H\033[2J");

		err = Send_LoginPacket();

		if(err == ERR_NONE){
			fprintf(stderr, "Send Login Packet ... OK\n");
		}else{
			fprintf(stderr, "Send Login Packet ... ERROR\n");
		}

		for(i=0;i<800;i++){
			usleep(10000);
		}
	
		fprintf(stderr, "Read packet info ...\n");

		GSM_GetPacketInfo(&unread_sum, &packet_sum);

		if(unread_sum>0){
			while(unread_sum>0){
			unread_sum--;
			memset(buff,'\0',1024);
			UDP_ReceivePacket(&link_num, &data_index, &data_len, buff);	
			
			fprintf(stderr, "Receive Packet ...\n");
			fprintf(stderr, "link num    = %d\n",link_num);
			fprintf(stderr, "data index  = %d\n",data_index);
			fprintf(stderr, "data length = %d\n",data_len);

			CRC_Receive  = (buff[0]<<8) + buff[1];
			CRC_Caculate = Caculate(&buff[2],data_len-2)&0xFFFF;				
			fprintf(stderr, "CRC = 0x%04X\n",CRC_Caculate);
			if(CRC_Receive == CRC_Caculate){
				fprintf(stderr,"CRC is OK\n");
			}else{
				fprintf(stderr,"CRC is ERROR\n");
			}
			
			fprintf(stderr, "\n > ");

			for(i=0; i<data_len; i++){
				fprintf(stderr, "0x%02X ", buff[i]);
			}	
			fprintf(stderr, "\n");

			if(APP_ERR_NONE==Wait_LoginACK(buff)){
				is_LogIn = 1;
				break;
			}

			fprintf(stderr, "\n++++++++++ ==== +++++++++\n");
			}
		} else{
			fprintf(stderr, "OK\n");
		}
		
		if(is_LogIn==1){
			fprintf(stderr, "\nDevice Login success!\n");
			break;
		}

		for(i=0;i<3000;i++){
			usleep(10000);
		}
	}
}

void NetWork_Connection_Config(void)
{
	// config the gprs link
	GSM_Reset();
	GSM_Config();

	GSM_NetworkConfirm();
	usleep(10000);
	GSM_GprsSetup();
	
	fprintf(stderr, "**** setup tcpip ****\n");	
	usleep(10000);
	GSM_TcpipSetup();
	GSM_SetupUDPLink("211.137.45.80", "11004", "4098");
}
