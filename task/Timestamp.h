/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-10-18 09:30
#      Filename : Timestamp.h
#   Description : not thing...
#
=============================================================================*/

#ifndef _TIME_STAMP_H_
#define	_TIME_STAMP_H_

#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <errno.h>
#include <time.h>

#define		_Basic_Stamp	946684800ul

//#define		_Basic_Stamp	0ul

void GetTimestamp(uint32_t *seconds);

#endif

