/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-10-18 09:41
#      Filename : RectJudge.c
#   Description : not thing...
#
=============================================================================*/

#include "PolyJudge.h"

static EarthPos	R_CurPoint;
static EarthPos R_Corners[4];
static int		R_CornerNum = 4;

void PosNormalize(Rectangle_Struct *rect, GpsInfo *pGps)
{
	EarthPos TL_Corner;
	EarthPos BR_Corner;

	MSEC2Deg(rect->Top_left_corner.Latitude,     rect->Top_left_corner.Longitude,     &TL_Corner);
	MSEC2Deg(rect->Bottom_right_corner.Latitude, rect->Bottom_right_corner.Longitude, &BR_Corner);
	
	R_Corners[0].lat = TL_Corner.lat;
	R_Corners[0].lon = TL_Corner.lon;
	
	R_Corners[1].lat = TL_Corner.lat;
	R_Corners[1].lon = BR_Corner.lon;	
	
	R_Corners[2].lat = BR_Corner.lat;
	R_Corners[2].lon = BR_Corner.lon;

	R_Corners[3].lat = BR_Corner.lat;
	R_Corners[3].lon = TL_Corner.lon;
	
	R_CornerNum = 4;
	
	NDEG2Deg(pGps->lat, pGps->lon, &R_CurPoint);
}


int IsPosIn_Rect(Rectangle_Struct *rect, GpsInfo *pGps)
{
	PosNormalize(rect, pGps);
	return ZoneDetect(&R_CurPoint, R_CornerNum, R_Corners);
}
