/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-10-21 11:13
#      Filename : TimeCompare.c
#   Description : not thing...
#
=============================================================================*/

#include <stdio.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <errno.h>
#include <time.h>

#include "TimeCompare.h"
#include "PositionUpdateRule.h"

int Is_BeyondTime(Time_DT *ptime)
{
	double 		diff;
	time_t 		current_time;
	struct tm 	TimeInput;
	time_t		time_compare;

	TimeInput.tm_sec  = ptime->second;
	TimeInput.tm_min  = ptime->minute;
	TimeInput.tm_hour = ptime->hour;
	TimeInput.tm_mday = ptime->day;
	TimeInput.tm_mon  = (ptime->month) - 1;
	TimeInput.tm_year = (2000+ptime->year) - 1900;

	time_compare = mktime(&TimeInput);
	
	// get current time
	time(&current_time);
	
	diff = difftime(current_time, time_compare);

	if(diff>0.0) 
		return 1;
	else 
		return 0;
}



int Is_BeyondTime_D(Time_D *ptime)
{
	double 		diff;
	time_t 		current_time;
	struct tm 	*Ref_Time;
	struct tm 	TimeInput;
	time_t		time_compare;


	time(&current_time);
	Ref_Time = localtime(&current_time);

	memcpy((char *)&TimeInput, (char *)Ref_Time, sizeof(struct tm));

	TimeInput.tm_sec  = ptime->second;
	TimeInput.tm_min  = ptime->minute;
	TimeInput.tm_hour = ptime->hour;

	time_compare = mktime(&TimeInput);
	
	// get current time
	time(&current_time);
	
	diff = difftime(current_time, time_compare);

	if(diff>0.0) 
		return 1;
	else 
		return 0;
}
