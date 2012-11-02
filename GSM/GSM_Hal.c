/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-10-18 09:36
#      Filename : GSM_Hal.c
#   Description : not thing...
#
=============================================================================*/

#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/file.h>
#include <sys/time.h>
#include <string.h>
#include <termios.h>
#include <errno.h>
#include <assert.h>

#include "GSM_Hal.h"
#include "AtTransmit.h"

#define _GSM_HAL_DEBUG_

#ifdef _GSM_HAL_DEBUG_
void Debug_ShowMsg(const char * cmd, char *rxbuff,size_t bytes)
{
	fprintf(stderr, "============================\n");
	fprintf(stderr,"%s",cmd);
	fprintf(stderr,"%s\n",rxbuff);
	fprintf(stderr,"Receive bytes :%d\n",bytes);
	fprintf(stderr, "============================\n");
}
#else
void Debug_ShowMsg(const char * cmd, char *rxbuff,size_t bytes)
{
}
#endif


GSM_Error GSM_Reset(void)
{
	GSM_Error res;
	char buff[64] = {'\0'};
	int  bytes;
	int  i;
	
	const char sRST[] = "AT%RST\r\n";		// expect : AT-Command Interpreter ready
	
	AT_Send(sRST,sizeof(sRST)-1);
	
	memset(buff,'\0',64);	
	i = 0;
	do{
		i++;
		usleep(100000);		// 100ms
		bytes = AT_Receive(buff);
	}while((i<30)&&(bytes==0));
	
	Debug_ShowMsg(sRST,buff,bytes);
	
	if(0 != strstr(buff,"AT-Command Interpreter ready\r\n")){
		fprintf(stderr,"EM310 Reset...Success\n");
		res = ERR_NONE;
	} else 
	{
		fprintf(stderr,"EM310 Reset...Error\n");
		res = ERR_UNKNOWN;
	}
	
	return res;
}


GSM_Error GSM_Config(void)
{
	char buff[32] = {'\0'};
	int  bytes;
	int  res;
	
	const char xECHO[] = "ATE0\r\n";		// expect : OK
	const char gCGMI[] = "AT+CGMI\r\n";		// expect : HUAWEI
	const char gCGMM[] = "AT+CGMM\r\n";		// expect : EM310
	const char gTSIM[] = "AT%TSIM\r\n";		// expect : %TSIM 1
	
	//============================================
	// turn of the echo
	do{
		AT_Send(xECHO,sizeof(xECHO)-1);
	
		memset(buff,'\0',32);	
		bytes = AT_Receive(buff);
	
		Debug_ShowMsg(xECHO,buff,bytes);	
	
		if(0 != strstr(buff,"OK\r\n")){  
			fprintf(stderr,"ATE0...success\n");
			res = ERR_NONE;
		}
		else{
			fprintf(stderr,"ATE0...Error\n");
			res = ERR_UNKNOWN;
		}
	}while(res!=ERR_NONE);

	//============================================
	// get the manufacturer
	AT_Send(gCGMI,sizeof(gCGMI)-1);
	
	memset(buff,'\0',32);	
	bytes = AT_Receive(buff);
	
	Debug_ShowMsg(gCGMI,buff,bytes);
	
	if(0 != strstr(buff,"HUAWEI\r\n\r\nOK\r\n")){  
		fprintf(stderr,"AT+CGMI...success\n");
		
	}
	else{
		fprintf(stderr,"AT+CGMI...Error\n");
		return ERR_UNKNOWN;
	}

	//============================================
	// get the mode num
	AT_Send(gCGMM,sizeof(gCGMM)-1);
	
	memset(buff,'\0',32);	
	bytes = AT_Receive(buff);
	
	Debug_ShowMsg(gCGMM,buff,bytes);
	
	if(0 != strstr(buff,"EM310\r\n\r\nOK\r\n")){  
		fprintf(stderr,"AT+CGMM...success\n");
		fprintf(stderr,"EM310 confirm...\n");

	}
	else{
		fprintf(stderr,"AT+CGMM...Error\n");
		return ERR_UNKNOWN;
	}

	//============================================
	// get the mode num
	AT_Send(gTSIM,sizeof(gTSIM)-1);
	
	memset(buff,'\0',32);	
	bytes = AT_Receive(buff);
	
	Debug_ShowMsg(gTSIM,buff,bytes);
	
	if(0 != strstr(buff,"%TSIM 1\r\n\r\nOK\r\n")){  
		fprintf(stderr,"AT%%TSIM...success\n");
		fprintf(stderr,"SIM card is insert...\n");

	}
	else{
		fprintf(stderr,"AT+CGMM...Error\n");
		fprintf(stderr,"SIM card is not insert...\n");
		return ERR_UNKNOWN;
	}

	return ERR_NONE;
}


GSM_Error GSM_GetCSQ(int *CSQ)
{
	char buff[32] = {'\0'};
	int  bytes;
	int  i,len,tmp;
	char *pstart;
	char *pend;
	
	const char gCSQ[] = "AT+CSQ\r\n";		// expect : OK
	
	//============================================
	// turn of the echo
	AT_Send(gCSQ,sizeof(gCSQ)-1);
	
	memset(buff,'\0',32);	
	bytes = AT_Receive(buff);
	
	Debug_ShowMsg(gCSQ,buff,bytes);	
	
	if(0 != strstr(buff,"OK\r\n")){
		if(0 == strstr(buff,"ERROR")){
			fprintf(stderr,"AT+CSQ...success\n");
			
			pstart = strstr(buff,": ")+2;
			pend   = strstr(buff,",");
			len    = pend - pstart;
	
			tmp = 0;
			for(i=0; i<len; i++){
				tmp = tmp*10 + *(pstart)-'0';
				pstart++;
			}
			*CSQ = tmp;
		}
	}
	else{
		fprintf(stderr,"AT+CSQ...Error\n");
		return ERR_UNKNOWN;
	}
	
	return ERR_NONE;
}

GSM_Error GSM_NetworkConfirm(void)
{
	GSM_Error res;

	do{
		sleep(2);
		res = GSM_NetworkTest();
	}while(res!=ERR_NONE);

	return ERR_NONE;
}


GSM_Error GSM_NetworkTest(void)
{
	char buff[64] = {'\0'};
	int  bytes;
	
	const char sCREG [] = "AT+CREG=1\r\n";		// Network Registration -- expect : OK	
	const char gCOPS [] = "AT+COPS?\r\n";		// Operator Selection   -- expect : CHINA MOBILE

	//============================================
	// Network Registration
	AT_Send(sCREG,sizeof(sCREG)-1);
	
	memset(buff,'\0',64);	
	bytes = AT_Receive(buff);
	
	Debug_ShowMsg(sCREG,buff,bytes);	
	
	if(0 != strstr(buff,"OK\r\n")){  
		fprintf(stderr,"Network Registration...success\n");
	}
	else{
		fprintf(stderr,"Network Registration...Error\n");
		return ERR_UNKNOWN;
	}
	
	usleep(2000);

	//============================================
	// Operator Selection
	AT_Send(gCOPS,sizeof(gCOPS)-1);
	
	memset(buff,'\0',64);	
	bytes = AT_Receive(buff);
	
	Debug_ShowMsg(gCOPS,buff,bytes);	
	
	if(0 != strstr(buff,"CHINA  MOBILE")){  
		fprintf(stderr,"Operator Selection : CHINA MOBILE\n");
	}
	else{
		fprintf(stderr,"Operator Selection...Error\n");
		return ERR_UNKNOWN;
	}
	
	return ERR_NONE;
}


GSM_Error GSM_GprsSetup(void)
{
	char buff[128] = {'\0'};
	int  bytes;
	
	int	 Attach = 0;
	int	 Register = 0;

	const char gCGATT[] = "AT+CGATT?\r\n";		// Attach /Detach From GPRS Service -- expect : +CGATT: 1
	const char sCGATT[] = "AT+CGATT=1\r\n";		// Attach /Detach From GPRS Service -- expect : OK
	const char gCGREG[] = "AT+CGREG?\r\n";		// check GPRS Registration status -- expect : +CGREG: 1,1	
	const char sCGREG[] = "AT+CGREG=1\r\n";		// GPRS Register -- expect : OK	

	//============================================
	// get the GPRS Service state
	AT_Send(gCGATT,sizeof(gCGATT)-1);
	
	memset(buff,'\0',128);	
	bytes = AT_Receive(buff);
	
	Debug_ShowMsg(gCGATT,buff,bytes);	
	
	if(0 != strstr(buff,"+CGATT: 1\r\n")){  
		fprintf(stderr,"GPRS Service is Attach\n");
		Attach = 1;
	}
	else if(0 != strstr(buff,"+CGATT: 0\r\n")){
		fprintf(stderr,"GPRS Service is Detach\n");
		Attach = 0;
	} else {
		fprintf(stderr,"AT+CGATT?...Error\n");
		return ERR_UNKNOWN;
	}

	//============================================
	// Attach GPRS Service
	if( Attach==0 ){
		fprintf(stderr,"Try to Attach GPRS Service...\n");
		//========================================
		// Attach GPRS Service
		AT_Send(sCGATT,sizeof(sCGATT)-1);
		
		memset(buff,'\0',128);	
		bytes = AT_Receive(buff);
		
		Debug_ShowMsg(sCGATT,buff,bytes);	
		
		if(0 != strstr(buff,"OK\r\n")){  
			fprintf(stderr,"GPRS Service is Attach\n");
		} else {
			fprintf(stderr,"Can't Attach GPRS Service...Error\n");
			return ERR_UNKNOWN;
		}
	}

	//============================================
	// check GPRS Registration status
	AT_Send(gCGREG,sizeof(gCGREG)-1);
	
	memset(buff,'\0',128);	
	bytes = AT_Receive(buff);
	
	Debug_ShowMsg(gCGREG,buff,bytes);	
	
	if(0 != strstr(buff,"+CGREG: 1,")){  
		fprintf(stderr,"GPRS is Register\n");
		Register = 1;
	} else if(0 != strstr(buff,"+CGREG: 0,")) {
		fprintf(stderr,"GPRS is not Register\n");
		Register = 0;
	} else {
		fprintf(stderr,"AT+CGREG...ERROR\n");
		return ERR_UNKNOWN;
	}

	//============================================
	// GPRS Register
	if(Register == 0){
		fprintf(stderr,"Try to Register GPRS Service...\n");
		
		AT_Send(sCGREG,sizeof(sCGREG)-1);
		memset(buff,'\0',128);	
		bytes = AT_Receive(buff);
		Debug_ShowMsg(sCGREG,buff,bytes);	
		if(0 != strstr(buff,"OK\r\n")){  
			fprintf(stderr,"Register GPRS Service...success\n");
		}
		else{
			fprintf(stderr,"Register GPRS Service...Error\n");
			return ERR_UNKNOWN;
		}
	}

	return ERR_NONE;
}


GSM_Error GSM_TcpipSetup(void)
{
	GSM_Error res;
	char buff[128] = {'\0'};
	int  bytes;
	int  i=0;
	
	const char sIOMODE[] = "AT%IOMODE=1,1,0\r\n";				// convert the data 
																// signel AT connection
																// using the building buff
																// expect : OK
	const char sAPN[] = "AT+CGDCONT=1,\"IP\",\"CMNET\"\r\n";	// expect : ok

	//const char sAPN[] = "AT+CGDCONT?\r\n";	// expect : ok
	const char sENTCPIP[] = "AT%ETCPIP=\"user\",\"gprs\"\r\n";	// expect : ok
	const char gENTCPIP[] = "AT%ETCPIP?\r\n";					// check if the GPRS is initial success, expect : ok
	
	
	res = GSM_NetworkTest();
	if(res != ERR_NONE){
		fprintf(stderr,"No network connection...\n");
		return res;
	} else{
		fprintf(stderr,"network OK...\n");
	}
	
	res = GSM_GprsSetup();
	if(res != ERR_NONE){
		fprintf(stderr,"No GPRS connection...\n");
		return res;
	} else{
		fprintf(stderr,"GPRS connection OK...\n");
	}
	
	//============================================
	// I/O mode config
	AT_Send(sIOMODE,sizeof(sIOMODE)-1);
	
	memset(buff,'\0',128);	
	bytes = AT_Receive(buff);
	
	Debug_ShowMsg(sIOMODE,buff,bytes);	
	
	if(0 != strstr(buff,"OK\r\n")){  
		fprintf(stderr,"I/O mode config...success\n");
	} else {
		fprintf(stderr,"I/O mode config...Error\n");
		return ERR_UNKNOWN;
	}

	//============================================
	// setup the apn

	AT_Send(sAPN,sizeof(sAPN)-1);
	
	memset(buff,'\0',128);		
	i = 0;
	do{
		i++;
		usleep(100000);		// 100ms

		bytes = AT_Receive(buff);
	}while((i<30)&&(bytes==0));
	
	Debug_ShowMsg(sAPN,buff,bytes);	
	
	if(0 != strstr(buff,"OK\r\n")){  
		fprintf(stderr,"Setup APN...success\n");
	} else {
		fprintf(stderr,"Setup APN...Error\n");
		return ERR_UNKNOWN;
	}
	
	//============================================
	// enable tcpip
	AT_Send(sENTCPIP,sizeof(sENTCPIP)-1);
	
	memset(buff,'\0',128);	
	i = 0;
	do{
		i++;
		usleep(100000);		// 100ms
		bytes = AT_Receive(buff);
	}while((i<30)&&(bytes==0));

	Debug_ShowMsg(sENTCPIP,buff,bytes);	
	
	if(0 != strstr(buff,"OK\r\n")){  
		fprintf(stderr,"Enable TCP/IP stack...success\n");
	} else {
		fprintf(stderr,"Enable TCP/IP stack...Error\n");
		return ERR_UNKNOWN;
	}

	//============================================
	// initial tcpip
	AT_Send(gENTCPIP,sizeof(gENTCPIP)-1);
	
	memset(buff,'\0',128);	
	
	do{
		i++;
		usleep(100000);		// 100ms
		bytes = AT_Receive(buff);
	}while((i<30)&&(bytes==0));

	Debug_ShowMsg(gENTCPIP,buff,bytes);	
	
	if(0 != strstr(buff,"OK\r\n")){  
		fprintf(stderr,"TCP/IP initial...success\n");
	} else {
		fprintf(stderr,"TCP/IP initial...Error\n");
		return ERR_UNKNOWN;
	}
	
	return ERR_NONE;
}

GSM_Error GSM_SetupUDPLink(char *dest_ip, char *dest_port, char *local_port)
{
	int  bytes;
	int  i=0;
	
	const char IPOPEN_H[] = "AT%IPOPEN=\"UDP\",";	// open udp link
	const char sIPDDMODE[] = "AT%IPDDMODE=0\r\n";	// packet auto del mode
	char txBuffer[128] = {'\0'};
	char rxbuff[64] = {'\0'};
	
	strcat(txBuffer, IPOPEN_H);
	strcat(txBuffer, "\"");
	strcat(txBuffer, dest_ip);
	strcat(txBuffer, "\",");
	strcat(txBuffer, dest_port);
	strcat(txBuffer, ",,");
	strcat(txBuffer, local_port);
	strcat(txBuffer, "\r\n");

	//============================================
	// open UDP connection
	AT_Send(txBuffer,strlen(txBuffer));
	
	memset(rxbuff,'\0',64);	
	
	i = 0;
	do{
		i++;
		usleep(100000);		// 100ms
		bytes = AT_Receive(rxbuff);
	}while((i<30)&&(bytes==0));
	
	Debug_ShowMsg(txBuffer,rxbuff,bytes);	

	if((0 != strstr(rxbuff,"CONNECT\r\n"))||(0 != strstr(rxbuff,"OK\r\n"))){  
		fprintf(stderr,"Open UDP Link...success\n");
	} else {
		fprintf(stderr,"Open UDP Link...Error\n");
		return ERR_UNKNOWN;
	}
	
	//============================================
	// set the packet del mode
	AT_Send(sIPDDMODE,strlen(sIPDDMODE));
	
	memset(rxbuff,'\0',64);	
	bytes = AT_Receive(rxbuff);
	Debug_ShowMsg(sIPDDMODE,rxbuff,bytes);	
	
	if(0 != strstr(rxbuff,"OK\r\n")){
		fprintf(stderr,"Config Packet auto del mode...success\n");
	} else {
		fprintf(stderr,"Config Packet auto del mode...Error\n");
		return ERR_UNKNOWN;
	}	
	
	
	return ERR_NONE;
}


GSM_Error GSM_SendPacket(char *pBuff)
{
	const char IPSEND_H[] = {"AT%IPSEND=\""};
	char TxBuff[2048] = {'\0'};
	GSM_Error res;
	char buff[64] = {'\0'};
	int  bytes;
	int  i = 0;
	
	strcat(TxBuff, IPSEND_H);
	strcat(TxBuff, pBuff);
	strcat(TxBuff,"\"\r\n");
	
	//============================================
	// Send packet
	AT_Send(TxBuff,strlen(TxBuff));
	
	memset(buff,'\0',64);	
	
	i = 0;
	do{
		i++;
		usleep(100000);		// 100ms
		bytes = AT_Receive(buff);
	}while((i<30)&&(bytes==0));
	
	Debug_ShowMsg("AT%IPSEND\r\n",buff,bytes);

	if(0 != strstr(buff,"OK\r\n")){
		res = ERR_NONE;
#ifdef _GSM_HAL_DEBUG_
		fprintf(stderr,"%s : Send Packet...success\n",__func__);
#endif
	} else {
		res = ERR_UNKNOWN;
#ifdef _GSM_HAL_DEBUG_
		fprintf(stderr,"Send Packet...Error\n");
#endif
	}
	
	fprintf(stderr,"%s : Send Packet...over\n",__func__);
	return res;
}

GSM_Error GSM_GetPacketInfo(char *pUnread_sum, char *pSum)
{
	GSM_Error res;
	char buff[128] = {'\0'};
	int  bytes, i, len = 0;
	char *pstart = NULL;
	char *pend = NULL;
	char tmp;
	
	const char gIPDQ[] = "AT%IPDQ\r\n";
	
	(*pUnread_sum) = 0;
	(*pSum) = 0;

	AT_Send(gIPDQ,strlen(gIPDQ));
	memset(buff,'\0',128);	
	bytes = AT_Receive(buff);

	if(bytes < 0) return ERR_NONE;
	//Debug_ShowMsg(gIPDQ,buff,bytes);
	
	if(0 != strstr(buff,"OK\r\n")){
			
			// get the unread_sum
			pstart = strstr(buff,"%IPDQ:");
			
			if(( pstart!=NULL )&&( pstart >= buff )&&( pstart < (buff+128) )){
				pstart = pstart + 6;
				pend   = strstr(pstart,",");
				len    = pend - pstart;
			}else{
			
				return ERR_NONE;
			}

			assert((len<10)&&(len>0));

			tmp = 0;
			for(i=0; i<len; i++){
				if((*pstart)!=' '){					// incase the space before the num
					tmp = tmp*10 + *(pstart)-'0';
				}
				pstart++;
			}
			*pUnread_sum = tmp;
			
			// get the packet sum
			pstart = pend + 1;
			pend   = strstr(pstart,"\r\n");
			if(pend == NULL){
				return ERR_NONE;
			}

			len    = pend - pstart;
			assert((len<10)&&(len>0));

			tmp = 0;
			for(i=0; i<len; i++){
				if((*pstart)!=' '){					// incase the space before the num
					tmp = tmp*10 + *(pstart)-'0';
				}
				pstart++;
			}
			*pSum = tmp;
			
			res = ERR_NONE;
	} else {

		AT_ClearUartFIFO();
		res = ERR_UNKNOWN;
	}
	
	return res;
}



// changeLog : fix the pData_len, the actual data length is 2*pData_len!!!

GSM_Error GSM_ReceivePacket(int *pLink_num, int *pData_index, int *pData_len, char *pBuff)
{
	const char gIPDR[] = "AT%IPDR=0\r\n";

	char buff[1024] = {'\0'};
	int  bytes, i;
	char *pstart;
	int  length;
	int  tmp;
	int  num_count = 0;

	AT_Send(gIPDR,strlen(gIPDR));
	memset(buff,'\0',1024);	
	bytes = AT_Receive(buff);

		
	fprintf(stderr,"buff add = %d\n",(int)buff);
	Debug_ShowMsg(gIPDR,buff,bytes);

	if(0 != strstr(buff,"OK\r\n")){
		
		// get packet info
		pstart = strstr(buff,"IPDR:")+5;
	
		fprintf(stderr,"pstart add = %d\n",(int)pstart);
		if((int)pstart!=5){
			
			// get the Link num
			tmp = 0;
			num_count = 0;
			while(((*pstart)!=',')&&(num_count<4)){
				if((*pstart)!=' '){
					tmp = tmp*10 + *(pstart)-'0';
				}
				pstart++;
				num_count++;
			}
			
			if(num_count>=4){
				fprintf(stderr,"%s : > 1... %d\n",__func__, num_count);
				*pData_len = 0;
				return ERR_UNKNOWN;
			}

			*pLink_num = tmp;
			
			// get the data index
			tmp = 0;
			num_count = 0;
			pstart++;
			while(((*pstart)!=',')&&(num_count<4)){
				if((*pstart)!=' '){
					tmp = tmp*10 + *(pstart)-'0';
				}
				pstart++;
				num_count++;
			}

			if(num_count>=4){
				fprintf(stderr,"%s : > 2... %d\n",__func__, num_count);
				*pData_len = 0;
				return ERR_UNKNOWN;
			}
			
			*pData_index = tmp;
		
			// get the data index	
			tmp = 0;
			num_count = 0;
			pstart++;
			while(((*pstart)!=',')&&(num_count<4)){
				if((*pstart)!=' '){
					tmp = tmp*10 + *(pstart)-'0';
				}
				pstart++;
				num_count++;
			}
			
			if(num_count>=4){
				fprintf(stderr,"%s : > 3... %d\n",__func__, num_count);
				*pData_len = 0;
				return ERR_UNKNOWN;
			}

			length = 2*tmp;
			*pData_len = tmp;
			fprintf(stderr,"%s : > length = %d\n",__func__, *pData_len);
		} else {
			fprintf(stderr,"%s : Packet1...Error\n",__func__);
			return ERR_UNKNOWN;
		}
		
		if(*pData_len>bytes){
			fprintf(stderr,"%s : Packet length parser...Error\n",__func__);
			return ERR_UNKNOWN;
		}
		
		// save the packet 
		pstart = strstr(buff,"\"")+1;
		if((int)pstart!=1){
			for(i=0;i<length;i++){
				*(pBuff) = *(pstart);
				pBuff++;
				pstart++;
			}
			*(pBuff) = '\0';
		} else {
			fprintf(stderr,"%s : Packet2...Error\n",__func__);
			return ERR_UNKNOWN;
		}
		
	} else {
		fprintf(stderr,"%s : Receive Packet...Error\n",__func__);
		return ERR_UNKNOWN;
	}
	
	return ERR_NONE;	
}


GSM_Error GSM_CloseConnection(void)
{
	GSM_Error res;
	char buff[64] = {'\0'};
	int  bytes;
	int  i;
	
	const char sCLOSETCPIP[] = "AT%IPCLOSE=1\r\n";		// expect : OK
	const char sCLOSEGPRS[]  = "AT%IPCLOSE=5\r\n";		// expect : OK

	//============================================
	// Close tcp/ip connection
	AT_Send(sCLOSETCPIP,sizeof(sCLOSETCPIP)-1);
	
	memset(buff,'\0',64);	
	i = 0;
	do{
		i++;
		usleep(100000);		// 100ms
		bytes = AT_Receive(buff);
	}while((i<30)&&(bytes==0));
	
	Debug_ShowMsg(sCLOSETCPIP,buff,bytes);
	
	if(0 != strstr(buff,"OK\r\n")){
		fprintf(stderr,"TCP/IP Connection Close...Success\n");
		res = ERR_NONE;
	} else 
	{
		fprintf(stderr,"TCP/IP Connection Close...Error\n");
		return ERR_UNKNOWN;
	}

	//============================================
	// Close GPRS connection
	AT_Send(sCLOSEGPRS,sizeof(sCLOSEGPRS)-1);
	
	memset(buff,'\0',64);	
	i = 0;
	do{
		i++;
		usleep(100000);		// 100ms
		bytes = AT_Receive(buff);
	}while((i<30)&&(bytes==0));
	
	Debug_ShowMsg(sCLOSEGPRS,buff,bytes);
	
	if(0 != strstr(buff,"OK\r\n")){
		fprintf(stderr,"GPRS Connection Close...Success\n");
		res = ERR_NONE;
	} else 
	{
		fprintf(stderr,"GPRS Connection Close...Error\n");
		return ERR_UNKNOWN;
	}
	
	return res;
}


