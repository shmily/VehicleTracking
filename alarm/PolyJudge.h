/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-10-18 09:42
#      Filename : PolyJudge.h
#   Description : not thing...
#
=============================================================================*/

#ifndef		_POLY_JUDGE_H_
#define		_POLY_JUDGE_H_

#include <stdint.h>
#include <stdio.h>

#include "ReportCondition.h"
#include "ZoneInfo.h"

int IsPosIn_Poly(Polygon_Struct *poly, GpsInfo *pGps);
int ZoneDetect(EarthPos *pCurPoint, int num, EarthPos *pCorners);

#endif
