/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-10-18 09:42
#      Filename : CircleJudge.c
#   Description : not thing...
#
=============================================================================*/

#include "CircleJudge.h"

nmeaPOS		CurPoint;
nmeaPOS 	Center;

//#define _debug_

void Circle_PosNormalize(Circle_Struct *circle, GpsInfo *pGps)
{
	double tmp;
	tmp          = nmea_ndeg2degree(pGps->lat);
	CurPoint.lat = nmea_degree2radian(tmp);
	
	tmp          = nmea_ndeg2degree(pGps->lon);
	CurPoint.lon = nmea_degree2radian(tmp);
	
	tmp        = (double)(circle->Center.Latitude)/_MSEC2DEG_FACTOR;
	Center.lat = nmea_degree2radian(tmp);
	
	tmp = (double)(circle->Center.Longitude)/_MSEC2DEG_FACTOR;
	Center.lon = nmea_degree2radian(tmp);
}

int IsPosIn_Circle(Circle_Struct *circle, GpsInfo *pGps)
{
	double distance;
	
	Circle_PosNormalize(circle, pGps);
	
	distance = nmea_distance(&Center, &CurPoint);
	
#ifdef _debug_
	fprintf(stderr, "distance = %f\n",distance);
#endif
	
	if((distance - circle->Radius) <= 0.0){
		return 1;
	}
	
	return 0;
}
