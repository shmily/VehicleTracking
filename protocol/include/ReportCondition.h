/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-10-18 10:00
#      Filename : ReportCondition.h
#   Description : not thing...
#
=============================================================================*/

#ifndef		_REPORT_CONDITION_H_
#define		_REPORT_CONDITION_H_


#include "AppLayer.h"
#include "myList.h"
#include "PositionUpdateRule.h"

// the max zone support
#define		_Max_Zone		8

// the max id length
#define		_RID_LEN		32

// action define
#define		_OUT_ZONE		0x0023
#define		_IN_ZONE		0x0022
#define 	_TD_SPEED		0x0008

// define the zone shape
#define		_Circle 		0x01
#define		_Rectangle  	0x02
#define		_Polygon 		0x03

// define the ack return poniter
typedef struct _ACK_Data_
{
	uint8_t		*pStart;
	uint8_t		*pEnd;
	uint8_t		ACK;
	void		*pCondition;
}ACK_DataPointer_Struct;


// point && shape defind
typedef struct _Point_
{
	uint8_t		location_Status;
	uint32_t	Latitude;
	uint32_t	Longitude;
	uint16_t	Alitude;
}Point_Struct;


typedef struct _Circle_Struct_
{
	Point_Struct 	Center;
	uint32_t	 	Radius;
}Circle_Struct;

typedef struct _Rectangle_Struct_
{
	Point_Struct 	Top_left_corner;
	Point_Struct	Bottom_right_corner;
}Rectangle_Struct;

typedef struct _Polygon_Struct_
{
	uint8_t			Corner_count;
	Point_Struct 	Corner[8];
}Polygon_Struct;


// time defined
typedef struct _Time_slot_
{
	Time_D		Btime;
	Time_D		Etime;
}TimeSlot_Struct;

typedef struct _Time_Set_
{
	uint8_t			TimeSet_Count;
	TimeSlot_Struct	BE_Time[6];
}TimeSet_Struct;


// about report condition
typedef struct _Report_Info_
{
	uint8_t		RID[_RID_LEN];
	uint8_t		R_len;
	uint8_t		Need_Confirm;
	uint8_t		On_now;
	uint8_t		Priority;
	uint16_t	Cnt;
	uint16_t	Interval;
	uint16_t	action;
}Report_Info;


typedef struct _IN_ZONE_Info_
{
	Report_Info 		Info;
	uint32_t			index;
	uint8_t				Shape;			// 0x01 : circle; 0x02 : rectangle;  0x03 : polygon
	void				*pShape_Value;
	TimeSet_Struct		TimeSet;
	struct list_head 	list;
}InZone_Report_Struct;

typedef struct _SpeedAnomaly_Report_
{
	Report_Info 		Info;
	uint32_t			index;
	uint8_t				speed;			// km/h
	TimeSet_Struct		TimeSet;
	uint16_t			Interval_t;
	struct list_head 	list;
}SpeedAnomaly_Report;


void R_SimpleAKV_Parser(uint8_t *pdata_start, uint8_t *pAttr, uint8_t *pK_len, uint16_t *pV_len, uint8_t **pKey, uint8_t **pValue);
void BeTimeAKV_Parser(uint8_t *pdata_start, uint8_t **pdata_end, TimeSet_Struct *pTimeSet);

APP_Error ReportConditionHandle(CommandType *command);
void InZoneCondition_Initial(struct list_head *phead);
void ShowCondition(struct list_head *phead);
void OutZoneCondition_Initial(struct list_head *phead);
void BEtimePrintf(TimeSlot_Struct *pTimeSlot);
#endif
