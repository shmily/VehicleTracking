/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-10-18 09:32
#      Filename : AtTransmit.c
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

#include "AtTransmit.h"
#include "../uart/uart.h"

static GSM_Device_Uart *GsmPort;

//
//=======================================================================
//
GSM_Error AtTransmitInit(GSM_Device_Uart *port)
{
	if(port->fd < 0) return ERR_DEVICENOTEXIST; 
	
	GsmPort = port;
	
	return ERR_NONE;
}

//
//======================================================================
//
void AT_ClearUartFIFO(void)
{
	serial_clearfifo(GsmPort);
}

//
//=======================================================================
//
int AT_Send(const void *buf, size_t nbytes)
{
	
	serial_clearfifo(GsmPort);
	usleep(20);
	return serial_write(GsmPort, buf, nbytes);
}


//
//=======================================================================
//
int AT_Receive(const void *buf)
{
	char * pbuff;
	int	   bytes;
	int    t_bytes;

	bytes = 0;
	t_bytes = 0;
	pbuff = (char *)buf;
	
	do{
		bytes = serial_read(GsmPort,pbuff,255);
		
		if(bytes<0) break;		
		
		t_bytes += bytes;
		if(bytes==4){
			if((*(pbuff)=='O')&&(*(pbuff+1)=='K')) break;
		}
		
		pbuff += bytes;
	}while(bytes>0);
	
	return t_bytes;
}

