/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-10-18 09:27
#      Filename : uart.h
#   Description : not thing...
#
=============================================================================*/

#ifndef __UART_H__
#define __UART_H__

#include <fcntl.h>
#include <sys/ioctl.h>
#include <termios.h>

#include "GSM-error.h"

typedef struct {
	int 			fd;
	struct termios 	old_settings;
} GSM_Device_Uart;

GSM_Error serial_close	   	(GSM_Device_Uart *port);
GSM_Error serial_setspeed  	(GSM_Device_Uart *port, int speed);
int 	  serial_read	   	(GSM_Device_Uart *port, void *buf, size_t nbytes);
int 	  serial_write	   	(GSM_Device_Uart *port, const void *buf, size_t nbytes);
int 	  serial_clearfifo 	(GSM_Device_Uart *port);
//int 	  serial_initial	(GSM_Device_Uart *port, int speed);
int 	  serial_initial	(char *pdev_str, GSM_Device_Uart *port, int speed);
#endif
