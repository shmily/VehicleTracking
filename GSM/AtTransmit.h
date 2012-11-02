/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-10-18 09:32
#      Filename : AtTransmit.h
#   Description : not thing...
#
=============================================================================*/

#ifndef __AT_TRANSMIT__
#define __AT_TRANSMIT__

#include "GSM-error.h"
#include "../uart/uart.h"

GSM_Error 	AtTransmitInit	(GSM_Device_Uart *port);
int 		AT_Send			(const void *buf, size_t nbytes);
int 		AT_Receive		(const void *buf);
void 		AT_ClearUartFIFO(void);

#endif
