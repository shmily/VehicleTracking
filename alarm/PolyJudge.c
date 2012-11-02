/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-10-18 09:42
#      Filename : PolyJudge.c
#   Description : not thing...
#
=============================================================================*/

#include "PolyJudge.h"

static EarthPos		P_CurPoint;
static EarthPos 	P_Corners[8];
static int			P_CornerNum = 0;

static void PosNormalize(Polygon_Struct *poly, GpsInfo *pGps)
{
	int num;
	int i;
	
	num = poly->Corner_count;
	P_CornerNum = num;
	
	for(i=0; i<num; i++){
		MSEC2Deg(poly->Corner[i].Latitude, poly->Corner[i].Longitude, &P_Corners[i]);
	}
	
	NDEG2Deg(pGps->lat, pGps->lon, &P_CurPoint);
}

int IsCrossLong180(int num, EarthPos *Corners)
{
	EarthPos *pCorners;
	double  prvLon;
	double  CurLon;
	int		i;
	
	if(num < 2){
		return -1;
	}
	
	pCorners = Corners;
	
	for(i=0; i<=num; i++){
		if(i==num){
			CurLon = pCorners->lon;
		}else{
			CurLon = (pCorners+i)->lon;
		}
		
		if(i!=0){
			if((CurLon*prvLon)<0.0){							// 如果相邻两点的经度值符号相反，则表明肯定跨0度或者180度经线
				if((fabs(prvLon) + fabs(CurLon)) >= 180.0){		// 如果绝对值之和大于180，那么就是跨180度经线(这里有个假设：两点间经度差小于180)
					return 1;
				}
			}
		}
		
		prvLon = CurLon;
	}
	
	return 0;
}

int ZoneDetect(EarthPos *pCurPoint, int num, EarthPos *pCorners)
{
	int			i;
	int			Cross180;
	int			nCrossNum = 0;
	
	EarthPos 	CurPoint;
	
	double		CurLon;
	double 		CurLat;
	double  	NetLon;
	double		NetLat;
	double		cal_lat;
	

	CurPoint.lat = pCurPoint->lat;
	CurPoint.lon = pCurPoint->lon;
	
	Cross180 = IsCrossLong180(num, pCorners);

	if((Cross180 == 1)&&(CurPoint.lon < 0)){
		CurPoint.lon += 360;
	}

	for(i=0; i<num; i++){
		CurLon = (pCorners+i)->lon;
		CurLat = (pCorners+i)->lat;
		
		if(i != num-1){					// 取得下一个点，组成一线段
			NetLon = (pCorners+i+1)->lon;
			NetLat = (pCorners+i+1)->lat;
		}else{							// 最后一点与第一点组成线段
			NetLon = pCorners->lon;
			NetLat = pCorners->lat;
		}
		
		if(Cross180 == 1){				// 如果穿过180度经线，负经度值需要转换为正经度值  
			if(CurLon < 0){
				CurLon += 360;
			}
			
			if(NetLon < 0){
				NetLon += 360;
			}
		}
		
		if((fabs(CurLon - CurPoint.lon)<0.000001)&&
		   (fabs(CurLat - CurPoint.lat)<0.000001)){		// 如果当前位置点就是顶点，则认为点在区域内
			return 1;
		}
		
		if((fabs(NetLon - CurPoint.lon)<0.000001)&&
		   (fabs(NetLat - CurPoint.lat)<0.000001)){		// 如果当前位置点就是顶点，则认为点在区域内
			return 1;
		}
		
		//如果两点中有一个点的纬度在90~待测点的纬度间，则满足纬度条件  
		if(((CurLat < 90)&&(CurLat > CurPoint.lat))||
		   ((NetLat < 90)&&(NetLat > CurPoint.lat))){
			
			// 继续判断是否满足纬度条件
			if(((CurPoint.lon < CurLon)&&(CurPoint.lon > NetLon))||
			   ((CurPoint.lon > CurLon)&&(CurPoint.lon < NetLon))){
				
				// 计算交点的纬度值
				cal_lat = CurLat - (CurLat - NetLat)*(CurLon - CurPoint.lon)/(CurLon - NetLon);
				
				// 如果纬度值就是待测点的纬度值，表明点在多边形边线上，认为在多边形内
				if(fabs(cal_lat - CurPoint.lat) < 0.000001){
					return 1;
				}
				
				//如果纬度值大于待测纬度值，表明两条线段是相交线段，计数加1
				if(cal_lat > CurPoint.lat){
					nCrossNum++;
				}	
			}
		}	
	}

	if((nCrossNum%2) == 1){
		return 1;
	}
	
	return 0;

}


int IsPosIn_Poly(Polygon_Struct *poly, GpsInfo *pGps)
{
	
	PosNormalize(poly, pGps);
	return ZoneDetect(&P_CurPoint, P_CornerNum, P_Corners);

}
