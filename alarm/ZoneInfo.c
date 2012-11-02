/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-10-18 09:41
#      Filename : ZoneInfo.c
#   Description : not thing...
#
=============================================================================*/

#include "ZoneInfo.h"

void NDEG2Deg(double lat, double lon, EarthPos *pout)
{
	pout->lat = nmea_ndeg2degree( lat );
	pout->lon = nmea_ndeg2degree( lon );
}


int NDEG2MSEC(double input)
{
	double deg;
	int	   result;

	deg = nmea_ndeg2degree( input );
	result = (int)(deg*60*60*1000);

	return result;
}

void MSEC2Deg(uint32_t lat, uint32_t lon, EarthPos *pout)
{
	pout->lat = (double)lat/_MSEC2DEG_FACTOR;
	pout->lon = (double)lon/_MSEC2DEG_FACTOR;
}

double MSEC2NDeg(int msdeg)
{
	double deg;
	deg = (double)msdeg/_MSEC2DEG_FACTOR;
	
	return nmea_degree2ndeg(deg);
}

uint32_t Deg2MSEC(double deg, double min, double sec)
{
	uint32_t tmp;

	tmp = (int)(deg*60*60*1000);
	tmp = tmp + (int)(min*60*1000);
	tmp = tmp + (int)(sec*1000);

	return tmp;	
}

double Deg2NDEG(double deg, double min, double sec)
{
	double tmp;

	tmp = deg*100 + min;
	tmp = tmp + sec/60;

	return tmp;
}


int Knot2Kilometer(double Knot)
{
	return (int)(Knot*NMEA_TUD_KNOTS);
}




