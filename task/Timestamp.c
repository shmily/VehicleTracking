/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-10-18 09:30
#      Filename : Timestamp.c
#   Description : not thing...
#
=============================================================================*/

#include "Timestamp.h"


void GetTimestamp(uint32_t *seconds)
{
	struct timeval	tv;
	struct timezone tz;
	uint32_t		Timestamp;
	
	gettimeofday(&tv, &tz);
	
	Timestamp = (uint32_t)tv.tv_sec;
	
	Timestamp = Timestamp - _Basic_Stamp;
	
	(*seconds) = Timestamp;
}
