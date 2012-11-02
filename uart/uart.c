/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-10-18 09:26
#      Filename : uart.c
#   Description : not thing...
#
=============================================================================*/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/file.h>
#include <sys/time.h>
#include <string.h>
#include <termios.h>
#include <errno.h>
#include <assert.h>

#include "uart.h"


typedef struct {
	speed_t code;
	int		value;
} baud_record;

#define SERIAL_DEFAULT_SPEED 9600

static baud_record baud_table[] = {
	{ B4800,	4800 },
	{ B9600,	9600 },
	{ B115200,	115200 },
	{ B0,		0 },
};

//
//=======================================================================
//
GSM_Error serial_close(GSM_Device_Uart *port)
{
	if (port->fd < 0) return ERR_NONE;

	/* Restores old settings */
	tcsetattr(port->fd, TCSANOW, &(port->old_settings));

	/* Closes device */
	close(port->fd);

	port->fd = -1;

	return ERR_NONE;
}

//
//=======================================================================
//
static GSM_Error serial_open (char *pdev_str, GSM_Device_Uart *port)
{
	struct termios t;

	/* O_NONBLOCK MUST is required to avoid waiting for DCD */
	port->fd = open(pdev_str, O_RDWR | O_NOCTTY | O_NONBLOCK);
	if (port->fd < 0) {
		return ERR_DEVICEOPENERROR;
	}

	/* open() calls from other applications shall fail now */
	ioctl(port->fd, TIOCEXCL, (char *) 0);

	if (tcgetattr(port->fd, &(port->old_settings)) == -1) {
		close(port->fd);
		return ERR_DEVICEOPENERROR;
	}

	if (tcflush(port->fd, TCIOFLUSH) == -1) {
		serial_close(port);
		return ERR_DEVICEOPENERROR;
	}

	/* Use previous settings as start */
	t = port->old_settings;

	/* Opening without parity */
	t.c_iflag       = IGNPAR;
	t.c_oflag       = 0;
	/* disconnect line, 8 bits, enable receiver,
	 * ignore modem lines,lower modem line after disconnect
	 */
//	t.c_cflag       = B0 | CREAD | CLOCAL | HUPCL;
	/* enable hardware (RTS/CTS) flow control (NON POSIX) 	*/
	/* t.c_cflag 	|= CRTSCTS;			  	*/
	t.c_lflag       = ICANON;
//	t.c_lflag &= ~ (ICANON | ECHO | ECHOE | ISIG);
	t.c_cc[VMIN]    = 200;
	t.c_cc[VTIME]   = 200;

	if (tcsetattr(port->fd, TCSANOW, &t) == -1) {
		serial_close(port);
		return ERR_DEVICEOPENERROR;
	}

	return ERR_NONE;
}

//
//=======================================================================
//
static GSM_Error serial_setparity(GSM_Device_Uart *port, int parity)
{
	struct termios	  	t;

	assert(port->fd >= 0);

	if (tcgetattr(port->fd , &t)) {
		return ERR_DEVICEPARITYERROR;
	}

	if (parity) {
		t.c_cflag |= (PARENB | PARODD);
		t.c_iflag = 0;
	} else {
		t.c_iflag = IGNPAR;
	}

	if (tcsetattr(port->fd, TCSANOW, &t) == -1){
		serial_close(port);
		return ERR_DEVICEPARITYERROR;
	}

	return ERR_NONE;
}

//
//=======================================================================
//
GSM_Error serial_setspeed(GSM_Device_Uart *port, int speed)
{
	struct termios  t;
	baud_record		*curr = baud_table;

	assert(port->fd >= 0);

	if (tcgetattr(port->fd, &t)) {
		return ERR_DEVICEREADERROR;
	}

	while (curr->value != speed) {
		curr++;
		/* This is how we make default fallback */
		if (curr->value == 0) {
			if (speed == SERIAL_DEFAULT_SPEED) {
				return ERR_NOTSUPPORTED;
			}
			curr = baud_table;
			speed = SERIAL_DEFAULT_SPEED;
		}
	}

	cfsetispeed(&t, curr->code);
	cfsetospeed(&t, curr->code);

	if (tcsetattr(port->fd, TCSADRAIN, &t) == -1) {
		serial_close(port);
		return ERR_DEVICECHANGESPEEDERROR;
	}

	return ERR_NONE;
}


//
//=======================================================================
//
int serial_read(GSM_Device_Uart *port, void *buf, size_t nbytes)
{
	struct timeval  	timeout2;
	fd_set	  			readfds;
	int	     			actual = 0;

	assert(port->fd >= 0);

	FD_ZERO(&readfds);
	FD_SET(port->fd, &readfds);

	timeout2.tv_sec     = 3;
	timeout2.tv_usec    = 50000;

	if (select(port->fd+1, &readfds, NULL, NULL, &timeout2)>0) {
		actual = read(port->fd, buf, nbytes);
	}
	return actual;
}


//
//=======================================================================
//
int serial_write(GSM_Device_Uart *port, const void *buf, size_t nbytes)
{
	int		     	ret;
	size_t		  	actual = 0;
	const unsigned char *buffer = (const unsigned char *)buf; /* Just to have correct type */

	assert(port->fd >= 0);

	do {
		ret = write(port->fd, buffer, nbytes - actual);
		if (ret < 0) {
			if (errno == EAGAIN) {
				usleep(1000);
				continue;
			}
	    	if (actual != nbytes) {
				fprintf(stderr, "Wanted to write %ld bytes, but %ld were written\n",(long)nbytes, (long)actual);
			}
	    		return actual;
		}
		actual  += ret;
		buffer  += ret;
	} while (actual < nbytes);
	return actual;
}

//
//=======================================================================
//
int serial_clearfifo(GSM_Device_Uart *port)
{
	if(tcflush(port->fd,TCIOFLUSH) < 0){
		return ERR_DEVICEREADERROR;
	}
	
	return ERR_NONE;	
}


//
//=======================================================================
//
int serial_initial(char *pdev_str, GSM_Device_Uart *port, int speed)
{
	GSM_Error res;
	
	fprintf(stderr, "Initial the %s ...\n",pdev_str);
	
	res = serial_open(pdev_str, port);
	if(res != ERR_NONE){
		fprintf(stderr, "Can't opend the uart!\n");
		return ERR_DEVICEOPENERROR;
	}

	res = serial_setparity(port,0);
	if(res != ERR_NONE){
		fprintf(stderr, "Can't set parity!\n");
		return ERR_DEVICEOPENERROR;
	}

	res = serial_setspeed(port,9600);
	if(res != ERR_NONE){
		fprintf(stderr, "Can't set speed!\n");
		return ERR_DEVICEOPENERROR;
	}
	
	return ERR_NONE;
}
