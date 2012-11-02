/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-10-08 13:37
#      Filename : main.c
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

#include "uart.h"
#include "AtTransmit.h"
#include "GSM-error.h"
#include "GSM_Hal.h"
#include "UDP_Lib.h"
#include "Login.h"

#include "GPS.h"

#include "ObjectRequest.h"

#include "PositionUpdateRule.h"
#include "ReportCondition.h"
#include "ReportCondition_Speed.h"
#include "TLP.h"
#include "AppLayer.h"
#include "Interval_Locate.h"
#include "RegionDetection.h"
#include "SpeedDetection.h"

#include "Task.h"
#include "Timestamp.h"
#include "SequenceNumber.h"

#include "PolyJudge.h"
#include "CircleJudge.h"
#include "ZoneInfo.h"

#include "ImageTransmit.h"
#include "Btype_Control.h"

#include "ExtendFounction.h"
#include "DrowsyDetect.h"
#include "FIFO.h"

#define	_EN_INITIAL_GSM_

extern int _Debug_Close;

// about GPRS
GSM_Device_Uart GsmDevice;
int				CSQ;
int				res;
GpsInfo         GPS_Msg;
GpsInfo         GPS_Debug;

// about packet info
int				G_PacketType = 0;
int 			Time_Out;

char			unread_sum;
char			packet_sum;

int 			link_num;
int				data_index;
int 			data_len;
char			gRxBuff[1024];

PacketInfo		APP_Packet;
Packet_Struct	Packet;

// about rule
struct list_head RuleList;
struct list_head InZone_List;
struct list_head OutZone_List;
struct list_head TDSA_List;

// time printf
time_t			Time;
char			T_str[32];
int				T_len;

// for image transmit
char			image_path[64];

// for debug
int				test_rule = 0;

int main(void)
{
	int i;	

	// uart initial
	fprintf(stderr, "This is a test for the Contex-A8...\n");
	serial_initial("/dev/ttySAC1", &GsmDevice, 9600);
	AtTransmitInit(&GsmDevice);
	usleep(10000);
	SEQ_Init();	

	// GPS uart init
	GPS_Init();

	// FIFO initial
	Open_ImageFIFO(ImageFIFO);	

#ifdef	_EN_INITIAL_GSM_
	// config the gprs network
	NetWork_Connection_Config();
#endif
	
	// printf the CSQ info
	GSM_GetPacketInfo(&unread_sum, &packet_sum);
	fprintf(stderr, "Unread packet sum = %d\n",unread_sum);
	fprintf(stderr, "total packet sum  = %d\n",packet_sum);
	
	if(GSM_GetCSQ(&CSQ)!=ERR_NONE){
		fprintf(stderr, "Can't get the CSQ...\n");
	} else{
		fprintf(stderr, "CSQ = %d\n",CSQ);
	}

	for(i=0;i<20;i++){
		usleep(10000);
	}		

	Login_Process(gRxBuff);


	fprintf(stderr, "========= start transmit =========\n");
	
	for(i=0;i<100;i++){
		usleep(10000);
	}


	// =====================================
	GPS_Debug.lat = MSEC2NDeg( (double)81678360 ); 
	GPS_Debug.lon = MSEC2NDeg( (double)409628128 );
	GPS_Debug.speed = 60;	
	GPS_InfoPrintf(&GPS_Debug);
	// =====================================



	// image updata process

	//ImageTransmit_init("/home/plg/linux.jpg");	

	// packet process
	fprintf(stderr,"\r\n****************************\r\n");
	PositionUpdateRule_Initial(&RuleList);
	InZoneCondition_Initial(&InZone_List);
	OutZoneCondition_Initial(&OutZone_List);
	TDSA_Condition_Initial(&TDSA_List);

	Interval_Locate_Init();				// initial the position update process
	ZoneInfo_Report_Init();				// initial the Zone information update process
	Speed_Report_Init();				// initial the speed information update process 

	// task initial
	Task_Init();	
	SetTimeOut(_Task_Heartbeat_,   6);	// execute every 60 seconds
	SetTimeOut(_Task_GPS_Display_, 6);	// execute every 60 seconds
	SetTimeOut(_Task_RE_Login_,   15);	// if we did not receive the heart beat ack in 150 seconds, re login...

	while(1){
		GSM_GetPacketInfo(&unread_sum, &packet_sum);
	
		G_PacketType = TypeX;	
		if(unread_sum>0){
			memset(gRxBuff,'\0',1024);
			UDP_ReceivePacket(&link_num, &data_index, &data_len, gRxBuff);	
			
			fprintf(stderr, "Receive Packet ...\n");
			fprintf(stderr, "link num    = %d\n",link_num);
			fprintf(stderr, "data index  = %d\n",data_index);
			fprintf(stderr, "data length = %d\n",data_len);

			Packet.length = data_len;
			memcpy(&(Packet.Data[0]), gRxBuff, data_len);	
			res = TLP_PacketDevide(&Packet, &APP_Packet, &G_PacketType);
			
			// A type input
			if((res == ERROR_NONE)&&(G_PacketType == TypeA)){
				
				InputCommandHandle(&APP_Packet);
				ShowRules(&RuleList);
				ShowCondition(&InZone_List);
				ShowCondition(&OutZone_List);
				ShowSpeedCondition(&TDSA_List);
			}
			
			// B type input
			if(G_PacketType == TypeB){
				ReportUpdata_Loop(0);
			}
			
			// D type input
			if(G_PacketType == TypeD){
 				ImageTransmit_loop(0);
			}
		}
	
		// check the fifo, if we got a new image, send it...	
		DrowsyImage_Check();
		DrowsyImage_Send();

		// if the image transmit is in working, the other update mesage is delay	
		if(Get_DT_State()==DT_Idle){		

			Interval_Locate_Check(&RuleList);									// position update process
			Interval_Locate_Updata(&GPS_Msg);

			ZoneInfo_Condition_Check(&InZone_List, &OutZone_List, &GPS_Msg);	// Zone information process
			ZoneInfo_Update(&GPS_Msg);

			Speed_Condition_Check(&TDSA_List,&GPS_Debug);
			SpeedReport_Update(&GPS_Debug);

			if( isTimeOut(_Task_Heartbeat_)==1 ){
				HeartBeat_Request(NULL);
				ClearTimeOut(_Task_Heartbeat_);	
			}

			if( isTimeOut(_Task_RE_Login_)==1 ){
			
				// reconfig the network
				NetWork_Connection_Config();
				Login_Process(gRxBuff);
				ClearTimeOut(_Task_RE_Login_);	
			}
		}

		// GPS information
		GPS_Read(&GPS_Msg);
		if( isTimeOut(_Task_GPS_Display_)==1 ){
			GPS_InfoPrintf(&GPS_Msg);
			ClearTimeOut(_Task_GPS_Display_);	
		}
		
		usleep(500000);
	}

	GSM_CloseConnection();

	for(i=0;i<100;i++){
		usleep(20000);
	}	

	GSM_Reset();	

	serial_close(&GsmDevice);
	
	return 0;
}
