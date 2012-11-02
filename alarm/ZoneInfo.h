/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-10-18 09:41
#      Filename : ZoneInfo.h
#   Description : not thing...
#
=============================================================================*/

#ifndef		_ZONE_INFO_H_
#define		_ZONE_INFO_H_

#include <stdint.h>
#include <stdio.h>
#include <math.h>

#include "nmea.h"
#include "GPS.h"

#define		_MSEC2DEG_FACTOR	3600000.0

typedef struct _erath_POS_
{
	double lat;
	double lon;
}EarthPos;


int NDEG2MSEC(double input);
void NDEG2Deg(double lat, double lon, EarthPos *pout);
void MSEC2Deg(uint32_t lat, uint32_t lon, EarthPos *pout);
double MSEC2NDeg(int msdeg);
uint32_t Deg2MSEC(double deg, double min, double sec);
double Deg2NDEG(double deg, double min, double sec);


int Knot2Kilometer(double Knot);

void isInZone();

#endif
