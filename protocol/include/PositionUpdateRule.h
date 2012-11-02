/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-10-18 09:59
#      Filename : PositionUpdateRule.h
#   Description : not thing...
#
=============================================================================*/

#ifndef _POS_UPDATE_RULE_H_
#define	_POS_UPDATE_RULE_H_

#include "AppLayer.h"
#include "myList.h"

#define _ID_LEN		32

// date time struct
typedef struct _Time_DT_
{
	uint8_t year;
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
}Time_DT;

// time pointer struct
typedef struct _Time_D_
{
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
}Time_D;

// date struct
typedef struct _Date_D_
{
	uint8_t year;
	uint8_t month;
	uint8_t day;
}Date_D;

// rule effective struct
typedef struct _Rule_Effective_
{
	uint8_t  AreMove;
	uint16_t Cnt;
	uint16_t Interval;
}
Rule_Effective;

typedef struct _Rule_Struct_
{
	uint8_t  			ID[_ID_LEN];
	Rule_Effective 		Effective;
	uint8_t				BMAP_Type;
	void *				BMAP;
	uint8_t				TMAP_Type;
	void *				TMAP;
	uint8_t				EMAP_Type;
	void *				EMAP;
	struct list_head 	list;
}Rule_Struct;

// rule struct 
typedef struct _Rule_Member_
{
	uint32_t Sent_cnt;		// "1" max send times
	uint16_t Speed;			// "2" speed , KMph
	uint16_t Speed_offset;	// "3" speed change, KMph
	uint16_t Direction;		// "4" direction, the north is 0
	uint16_t Dir_offset;	// "5" direction change
	Time_DT	 Time;			// "6" when reach the time
	uint16_t Time_change;	// "7" time change, second, =0 : not affect
	uint32_t Mileage;		// "8" when reach the mileage, KM
	uint32_t Distance;		// "9" m
}Rule_Member;

typedef struct _Rule_Collection_
{
	Rule_Member BMAP;		// begine rule map
	Rule_Member TMAP;		// interval rule map
	Rule_Member EMAP;		// end rule map
	uint16_t	BMAP_Mask;	// rule enable mask, 0 : disable; 1 : enable
	uint16_t	TMAP_Mask;	// rule enable mask, 0 : disable; 1 : enable
	uint16_t	EMAP_Mask;	// rule enable mask, 0 : disable; 1 : enable
}Rule_Collection;

typedef struct _Rule_ReqACK_Struct_
{
	uint8_t		id_length;
	uint8_t		id[_ID_LEN];
	uint8_t		Ack;			// 0x01:successed; 0x00:error
	uint8_t		AreMove;
	uint16_t	Cnt;
	uint16_t	Interval;
	uint8_t		bm_length;
	uint8_t		bm[64];
	uint8_t		tm_length;
	uint8_t		tm[64];
	uint8_t		em_length;
	uint8_t		em[64];
}Rule_ReqACK_info;


// function

void 	  PositionUpdateRule_Initial (struct list_head *phead);
APP_Error PositionUpdateRuleHandle	 (CommandType *command);
void 	  ShowRules					 (struct list_head *phead);
#endif
