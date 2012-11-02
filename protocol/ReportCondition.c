/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-10-11 10:34
#      Filename : ReportCondition.c
#   Description : not thing...
#
=============================================================================*/

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "ReportCondition.h"
#include "ReportCondition_Speed.h"
#include "ReportConditionList.h"
#include "UDP_Lib.h"
#include "crc16.h"

#define		_ix_AKV					((('i'<<8)|('x')) & 0xFFFF)
#define		_rT_AKV					((('r'<<8)|('T')) & 0xFFFF)
#define		_rS_AKV					((('r'<<8)|('S')) & 0xFFFF)

//#define		_Optional_AKV_EN_		0x00FF 		// enable the Optional AKV ACK

struct list_head *pInZoneCondition_List;
struct list_head *pOutZoneCondition_List;

// buff the pointer of the return data
ACK_DataPointer_Struct	InZone_ACK_DataPointer;
ACK_DataPointer_Struct	OutZone_ACK_DataPointer;

// static function defind
static void PointAKV_Parser(uint8_t *pdata_start, uint8_t **pdata_end, Point_Struct *pPoint);
static void ShapeValue_malloc(uint8_t shape, uint8_t *pStart, uint8_t **pEnd, void **pSave_node);
static void CircleAKV_Parser(uint8_t *pdata_start, uint8_t **pdata_end, Circle_Struct *pCircle);
static void RectangleAKV_Parser(uint8_t *pdata_start, uint8_t **pdata_end, Rectangle_Struct *pRectangle);
static void PolygonAKV_Parser(uint8_t *pdata_start, uint8_t **pdata_end, int length, Polygon_Struct *pPolygon);
static void TimeSetAKV_Parser(uint8_t *pdata_start, uint8_t **pdata_end, TimeSlot_Struct *pTimeSlot);
static void EnPacket_DefaultAckAKV(uint8_t *pdata, InZone_Report_Struct *pCondition, uint16_t *pLength);
static APP_Error InZone_Report(CommandType *command);
static APP_Error InZone_Report_ACK(CommandType *command);
static APP_Error OutZone_Report_ACK(CommandType *command);
static APP_Error OutZone_Report(CommandType *command);

// add by shmily --2012.09.26
static void ix_AKV_Paser(uint8_t * pData_start, uint32_t *pIndex, uint8_t **pData_end);
static void rT_AKV_Paser(uint8_t * pData_start, uint8_t *pShape, uint8_t **pData_end);
static void rS_AKV_Paser(uint8_t * pData_start, uint8_t Shape, void **pSave_node, uint8_t **pData_end);
static void  T_AKV_Paser(uint8_t * pData_start, TimeSet_Struct *pTimeSet, uint8_t **pData_end);

// show message
static void Condition_Printf(InZone_Report_Struct *pCondition);
static void CirclePrintf(Circle_Struct *pCircle);
static void RectanglePrintf(Rectangle_Struct *pRectangle);
static void PolygonPrintf(Polygon_Struct *pPolygon);

void InZoneCondition_Initial(struct list_head *phead)
{
	INIT_LIST_HEAD(phead);
	pInZoneCondition_List = phead;
}

void OutZoneCondition_Initial(struct list_head *phead)
{
	INIT_LIST_HEAD(phead);
	pOutZoneCondition_List = phead;
}


APP_Error ReportConditionHandle(CommandType *command)
{
	APP_Error res;

	// 
	if(command->Action == _IN_ZONE){
		DEBUG("%s : This is a in zone report set packet...\n",__func__);

		res = InZone_Report(command);

		if(res==APP_ERR_NONE) {
			res = InZone_Report_ACK(command);
		}
	} else if(command->Action == _OUT_ZONE){	
		DEBUG("%s : This is a out zone report set packet...\n",__func__);
		res = OutZone_Report(command);
		
		if(res == APP_ERR_NONE) {
			res = OutZone_Report_ACK(command);
		}
	} else if(command->Action == _TD_SPEED){	
		DEBUG("%s : This is a time depend Speed Anomaly report set packet...\n",__func__);
		res = TimeDepend_SpeedAnomaly_Report(command);
		
		if(res == APP_ERR_NONE) {
			res = TDSA_Report_ACK(command);
		}
	}

	return res;

}


// ===================================================================================
// receive the data and parser 

static APP_Error InZone_Report(CommandType *command)
{
	uint8_t 	*pdata;				// ponit to the data buff
	uint16_t 	tmp;
	uint8_t 	i;			
	uint8_t 	id_len;				// id length
	uint8_t 	id_tmp[_RID_LEN];	// buff the id
	uint8_t 	Need_Confirm;		// if the packet ack need to add the rule ack
	uint8_t		On_now;				// if the Report condition to be open right now
	uint8_t		Priority;			
	uint16_t	Cnt;
	uint16_t	Interval;
	uint32_t	Report_index;
	uint8_t		Shape;
	
	uint8_t		*pData_start;
	uint8_t		*pData_end;
	uint8_t		*pPacket_end;
	int			key_len;
	int			key_str;
	void        *pShape_Value;		// pointer to the memery of shape value
	TimeSet_Struct TimeSet_i;		// time set
	
	InZone_Report_Struct *pCondition;

	pdata = command->pData;

	// get the report id_len and id
	id_len = *(pdata++);
	DEBUG("%s : [id_len = %d]\n",__func__,id_len);
	assert(id_len <= _RID_LEN);		// we assert that the id_len < _RID_LEN
	memset(id_tmp,0,_RID_LEN);		// clear the id str
	for(i=0;i<id_len;i++){
		id_tmp[i] = *(pdata++);
	}
	DEBUG("%s : [ID : %s]\n",__func__,id_tmp);


	// if the packet ack need to add the rule ack 
	Need_Confirm = *(pdata++);
	DEBUG("%s : Need_Confirm : [%d]\n",__func__,Need_Confirm);

	// get the on_now value
	On_now = *(pdata++);
	DEBUG("%s : On_now : [%d]\n",__func__,On_now);

	// get the Priority value
	Priority = *(pdata++);
	DEBUG("%s : Priority : [%d]\n",__func__,Priority);

	// get the Cnt value
	tmp = *(pdata++);
	tmp = ((tmp<<8)&0xFF00) + *(pdata++);
	Cnt = tmp;
	DEBUG("%s : Cnt : [%d]\n",__func__,Cnt);

	// get the Interval value
	tmp = *(pdata++);
	tmp = ((tmp<<8)&0xFF00) + *(pdata++);
	Interval = tmp;
	DEBUG("%s : Interval : [%d]\n",__func__,Interval);
	
	// 解析可选可选参数，参数顺序不定，要特别注意~~
	InZone_ACK_DataPointer.pStart = pdata;												// we need this for ack
	pPacket_end = command->pData + command->length; 	
	DEBUG("%s : pPacket End address : [ 0x%08X ]\n",__func__,(int)pPacket_end);
	pData_start = pdata;
	pData_end   = pdata;
	
	while(pData_start < pPacket_end){		// paser the data ,untill reach the end
		key_len = *(pData_start + 1) & 0x00FF;
		
		if(key_len==1) {
			key_str = *(pData_start + 2);
			if(key_str=='T'){
				T_AKV_Paser(pData_start, &TimeSet_i, &pData_end);
			} else {	
				DEBUG("%s : Key str : [ 0x%08X ]\n",__func__,key_str);
				DEBUG("%s : Paser the AKV...ERROR\n",__func__);
			}
		} else if(key_len==2) {
			key_str = *(pData_start + 2);
			key_str = (key_str<<8) + *(pData_start + 3);
			if(key_str==_ix_AKV){
				ix_AKV_Paser(pData_start, &Report_index, &pData_end);
			} else if(key_str==_rS_AKV) {
				rS_AKV_Paser(pData_start, Shape, &pShape_Value, &pData_end);
			} else if(key_str==_rT_AKV) {
				rT_AKV_Paser(pData_start, &Shape, &pData_end);
			} else {
				DEBUG("%s : Paser the AKV...ERROR\n",__func__);	
				DEBUG("%s : Key str : [ 0x%08X ]\n",__func__,key_str);
			}
		} else {
			DEBUG("%s : Paser the AKV...ERROR\n",__func__);		
			DEBUG("%s : pData End address : [ 0x%08X ]\n",__func__,(int)pData_end);
		}
		pData_start = pData_end;
	}
	
	
	if(ReportCondition_Search(Report_index, pInZoneCondition_List, &pCondition) != 1){		// we can't find the report condition
		pCondition = malloc(sizeof(InZone_Report_Struct));						// malloc ram for the report condition
		memset(pCondition, 0, sizeof(InZone_Report_Struct));					// clear the rule
		ReportCondition_Add(&(pCondition->list),pInZoneCondition_List);
	} else {
		free(pCondition->pShape_Value);
	}

	// ---- save the value ----
	// save id
	memcpy(pCondition->Info.RID, id_tmp, _RID_LEN);
	pCondition->Info.R_len 		  = id_len;
	pCondition->Info.Need_Confirm = Need_Confirm;
	pCondition->Info.On_now 	  = On_now;
	pCondition->Info.Priority 	  = Priority;
	pCondition->Info.Cnt 		  = Cnt;
	pCondition->Info.Interval 	  = Interval;
	pCondition->Info.action 	  = command->Action;

	pCondition->index = Report_index;
	pCondition->Shape = Shape;

	pCondition->pShape_Value = pShape_Value;
	memcpy((char *)(&(pCondition->TimeSet)), (char *)&TimeSet_i, sizeof(TimeSet_Struct));

	InZone_ACK_DataPointer.pEnd = pData_end;											// we need this for ack
	InZone_ACK_DataPointer.pCondition = pCondition;

	InZone_ACK_DataPointer.ACK = 0x01;													// default, we send OK

	return APP_ERR_NONE;
}


void R_SimpleAKV_Parser(uint8_t *pdata_start, uint8_t *pAttr, uint8_t *pK_len, uint16_t *pV_len, uint8_t **pKey, uint8_t **pValue)
{
	uint8_t  *pdata;
	uint16_t tmp;
	
	pdata = pdata_start;
	
	(*pAttr)  =  *(pdata++);
	(*pK_len) =  *(pdata++);
	
	(*pKey) = pdata;			// save the key pointer
	
	pdata = pdata + (*pK_len);	// point to the V_len
	tmp = *(pdata++);
	tmp = (tmp<<8) + (*(pdata++));
	(*pV_len) = tmp;
	
	(*pValue) = pdata;			// save the Value pointer
}

static void ShapeValue_malloc(uint8_t shape, uint8_t *pStart, uint8_t **pEnd, void **pSave_node)
{
	void 				*p;
	uint8_t				*pAKV_end;
	Circle_Struct		*pCircle;
	Rectangle_Struct	*pRectangle;
	Polygon_Struct		*Polygon;

	// for AKV parser, no need to save
	uint8_t		Attr;
	uint8_t		K_len;
	uint16_t	V_len;
	uint8_t		*pKey;
	uint8_t		*pValue;

	assert(( shape >= _Circle ) && ( shape <= _Polygon ));

	// get the value address
	R_SimpleAKV_Parser(pStart, &Attr, &K_len, &V_len, &pKey, &pValue);
	assert(Attr == 0x11);
	assert(K_len == 2);			// "rS"

	switch (shape){

		case _Circle :
						p = malloc(sizeof(Circle_Struct));
						memset(p, 0, sizeof(Circle_Struct));
						pCircle = (Circle_Struct *)p;
						CircleAKV_Parser(pValue, &pAKV_end, pCircle);
						break;

		case _Rectangle :
						p = malloc(sizeof(Rectangle_Struct));
						memset(p, 0, sizeof(Rectangle_Struct));
						pRectangle = (Rectangle_Struct *)p;		
						RectangleAKV_Parser(pValue, &pAKV_end, pRectangle);
						break;
		case _Polygon : 
						p = malloc(sizeof(Polygon_Struct));
						memset(p, 0, sizeof(Polygon_Struct));
						Polygon = (Polygon_Struct *)p;		
						PolygonAKV_Parser(pValue, &pAKV_end, V_len, Polygon);
						break;
		default :		break;

	}

	(*pEnd) = pAKV_end;
	(*pSave_node) = p;
}


static void CircleAKV_Parser(uint8_t *pdata_start, uint8_t **pdata_end, Circle_Struct *pCircle)
{
	// for AKV parser, no need to save
	uint8_t		Attr;
	uint8_t		K_len;
	uint16_t	V_len;
	uint8_t		*pKey;
	uint8_t		*pValue;
	uint32_t	tmp;	

	uint8_t		*pPointAKV_start;
	uint8_t		*pPointAKV_end;

	// get the value address
	R_SimpleAKV_Parser(pdata_start, &Attr, &K_len, &V_len, &pKey, &pValue);
	assert(Attr == 0x10);
	assert(K_len == 1);			// "1"

	// save the center pointer data
	pPointAKV_start = pValue;
	PointAKV_Parser(pPointAKV_start, &pPointAKV_end, &(pCircle->Center));

	// get the radius
	R_SimpleAKV_Parser(pPointAKV_end, &Attr, &K_len, &V_len, &pKey, &pValue);
	assert(Attr == 0x00);
	assert(K_len == 1);					// "r"
	assert(V_len == 4);					// "r"
	tmp = 0;
	tmp = *(pValue++);
	tmp = (tmp<<8) + *(pValue++);
	tmp = (tmp<<8) + *(pValue++);
	tmp = (tmp<<8) + *(pValue++);
	pCircle->Radius = tmp;				// !!this line need to be fix, I don't know the value length!!

	(*pdata_end) = pValue;
}


static void RectangleAKV_Parser(uint8_t *pdata_start, uint8_t **pdata_end, Rectangle_Struct *pRectangle)
{
	uint8_t		*pPointAKV_start;
	uint8_t		*pPointAKV_end;

/*
	// get the value address
	R_SimpleAKV_Parser(pdata_start, &Attr, &K_len, &V_len, &pKey, &pValue);
	assert(Attr == 0x10);
	assert(K_len == 1);			// "2"
*/
	// save the Top_left_corner
	pPointAKV_start = pdata_start;
	PointAKV_Parser(pPointAKV_start, &pPointAKV_end, &(pRectangle->Top_left_corner));

	// save the Bottom_right_corner
	pPointAKV_start = pPointAKV_end;
	PointAKV_Parser(pPointAKV_start, &pPointAKV_end, &(pRectangle->Bottom_right_corner));

	(*pdata_end) = pPointAKV_end;
}


static void PolygonAKV_Parser(uint8_t *pdata_start, uint8_t **pdata_end, int length, Polygon_Struct *pPolygon)
{
	uint8_t		*pPointAKV_start;
	uint8_t		*pPointAKV_end;

	uint8_t		i;
	uint16_t 	len_iner;

/*
	// get the value address
	R_SimpleAKV_Parser(pdata_start, &Attr, &K_len, &V_len, &pKey, &pValue);
	assert(Attr == 0x10);
	assert(K_len == 1);			// "2"
*/

	
	DEBUG("%s : start to parser...\n",__func__);		
	len_iner = 0;
	pPointAKV_start = pdata_start;
	// the max is 8
	for(i=0; i<8; i++){
		DEBUG("%s : 0x%08X\n",__func__,(int)pPointAKV_start);		
		PointAKV_Parser(pPointAKV_start, &pPointAKV_end, &(pPolygon->Corner[i]));
		
		len_iner += (pPointAKV_end-pPointAKV_start);
		pPointAKV_start = pPointAKV_end;

		if(len_iner>=length) break;		
	}

	pPolygon->Corner_count = i+1;

	(*pdata_end) = pPointAKV_end;
}


static void PointAKV_Parser(uint8_t *pdata_start, uint8_t **pdata_end, Point_Struct *pPoint)
{
	// for AKV parser, no need to save
	uint8_t		Attr;
	uint8_t		K_len;
	uint16_t	V_len;
	uint8_t		*pKey;
	uint8_t		*pValue;

	uint8_t		*pLatitude_start;
	uint8_t		*pLongitude_start;

	uint32_t 	tmp;

	// get the value address
	R_SimpleAKV_Parser(pdata_start, &Attr, &K_len, &V_len, &pKey, &pValue);
	assert(Attr == 0x10);

/*
	pPointAKV_start = pValue;
	R_SimpleAKV_Parser(pPointAKV_start, &Attr, &K_len, &V_len, &pKey, &pValue);
	assert(Attr == 0x10);
*/
	// 经测试，不存在这个可选项	
	/*
	// get the status
	pStatus_start = pValue;
	R_SimpleAKV_Parser(pStatus_start, &Attr, &K_len, &V_len, &pKey, &pValue);
	assert(Attr == 0x00);
	assert(K_len == 1);			// "t"
	assert(V_len == 1);
	pPoint->location_Status = *(pValue++);
	*/


	// get the longitude
	pLongitude_start = pValue;
	R_SimpleAKV_Parser(pLongitude_start, &Attr, &K_len, &V_len, &pKey, &pValue);
	assert(Attr == 0x00);
	assert(K_len == 3);			// "lon"
	assert(V_len == 4);			// unint32_t type
	tmp = 0x00000000;
	tmp = *(pValue++);
	tmp = (tmp<<8) + *(pValue++);
	tmp = (tmp<<8) + *(pValue++);
	tmp = (tmp<<8) + *(pValue++);
	pPoint->Longitude = tmp;

	// get the latitude
	pLatitude_start = pValue;
	R_SimpleAKV_Parser(pLatitude_start, &Attr, &K_len, &V_len, &pKey, &pValue);
	assert(Attr == 0x00);
	assert(K_len == 3);			// "lat"
	assert(V_len == 4);			// unint32_t type
	tmp = 0x00000000;
	tmp = *(pValue++);
	tmp = (tmp<<8) + *(pValue++);
	tmp = (tmp<<8) + *(pValue++);
	tmp = (tmp<<8) + *(pValue++);
	pPoint->Latitude = tmp;
	
	// 不存在
	/*
	// get the alitude
	pAlitude_start = pValue;
	R_SimpleAKV_Parser(pAlitude_start, &Attr, &K_len, &V_len, &pKey, &pValue);
	assert(Attr == 0x00);
	assert(K_len == 3);			// "lon"
	assert(V_len == 2);			// unint16_t type
	tmp = 0x00000000;
	tmp = *(pValue++);
	tmp = (tmp<<8) + *(pValue++);
	pPoint->Alitude = (uint16_t)tmp;
	*/

	(*pdata_end) = pValue;		// point to the next AKV
}


void BeTimeAKV_Parser(uint8_t *pdata_start, uint8_t **pdata_end, TimeSet_Struct *pTimeSet)
{
	// for AKV parser, no need to save
	uint8_t		Attr;
	uint8_t		K_len;
	uint16_t	V_len;
	uint8_t		*pKey;
	uint8_t		*pValue;

	uint8_t		i;
	uint16_t 	len_iner;
	uint8_t		*pTimeAKV_start;
	uint8_t		*pTimeAKV_end;


	// get the value address
	R_SimpleAKV_Parser(pdata_start, &Attr, &K_len, &V_len, &pKey, &pValue);
	assert(Attr == 0x11);		// "T"	
	DEBUG("%s : V_len : [%d]\n",__func__, V_len);

	len_iner = 0;
	pTimeAKV_start = pValue;
	for(i=0; i<8; i++){
		TimeSetAKV_Parser(pTimeAKV_start, &pTimeAKV_end, &(pTimeSet->BE_Time[i]));
		
		len_iner += (pTimeAKV_end - pTimeAKV_start);
		pTimeAKV_start = pTimeAKV_end;

		if(len_iner >= V_len) break;
	}

	pTimeSet->TimeSet_Count = i+1;

	(*pdata_end) = pTimeAKV_end;
}


static void TimeSetAKV_Parser(uint8_t *pdata_start, uint8_t **pdata_end, TimeSlot_Struct *pTimeSlot)
{

	// for AKV parser, no need to save
	uint8_t		Attr;
	uint8_t		K_len;
	uint16_t	V_len;
	uint8_t		*pKey;
	uint8_t		*pValue;

	uint8_t		*pdata_iner;

	// get the value address
	R_SimpleAKV_Parser(pdata_start, &Attr, &K_len, &V_len, &pKey, &pValue);
	assert(Attr == 0x10);
	assert(K_len == 1);

	// get b time
	pdata_iner = pValue;
	R_SimpleAKV_Parser(pdata_iner, &Attr, &K_len, &V_len, &pKey, &pValue);
	assert(Attr == 0x00);
	assert(*pKey == 'e');
	assert(K_len == 1);
	pTimeSlot->Etime.hour   = *(pValue++);
	pTimeSlot->Etime.minute = *(pValue++);
	pTimeSlot->Etime.second = *(pValue++);

	// get e time
	pdata_iner = pValue;
	R_SimpleAKV_Parser(pdata_iner, &Attr, &K_len, &V_len, &pKey, &pValue);
	assert(Attr == 0x00);
	assert(*pKey == 'b');
	assert(K_len == 1);
	pTimeSlot->Btime.hour   = *(pValue++);
	pTimeSlot->Btime.minute = *(pValue++);
	pTimeSlot->Btime.second = *(pValue++);

	(*pdata_end) = pValue;
}


// ===================================================================================
// about the ack

static APP_Error InZone_Report_ACK(CommandType *command)
{

	GSM_Error		res;
	Packet_Struct   Packet;
	uint16_t		crc_resault;
	TLP_Head_Struct	*PacketHead;
	uint8_t			*pData;

	uint16_t 		defaultAKV_len;
	InZone_Report_Struct *pCondition;

	PacketHead = (TLP_Head_Struct *)&(Packet.Data[0]);
	pData      = (uint8_t *)&(Packet.Data[0]) + sizeof(TLP_Head_Struct);

	pCondition = (InZone_Report_Struct *)InZone_ACK_DataPointer.pCondition;

	// no need to change
	PacketHead->version = _VERSION;
	PacketHead->length  = sizeof(TLP_Head_Struct);
	PacketHead->type    = _TransmitCtrl;			// device to server
	PacketHead->medium  = _TransmitType;			// UDP
	PacketHead->encrypt = _Encrypt;					// encrypt none
	PacketHead->reserve = _Reserve;			
	
	PacketHead->DEV_IDH = HTONL(_DEV_IDH);
	PacketHead->DEV_IDL = HTONL(_DEV_IDL);
	
	//need to change
	PacketHead->timestamp = 0x00;					// need to fix
	PacketHead->SEQ_num = command->plowLevelInfo->SEQ_num;

	// send the ack
	*(pData++) = REPORT_SET;
	*(pData++) = (uint8_t)((_IN_ZONE>>8) & 0x00FF);
	*(pData++) = (uint8_t)(_IN_ZONE & 0x00FF);						// action

	*(pData++) = pCondition->Info.R_len;							// R_len;

	memcpy(pData, pCondition->Info.RID, pCondition->Info.R_len);	// RID
	pData = pData + pCondition->Info.R_len;

	*(pData++) = InZone_ACK_DataPointer.ACK;								// ACK

	if(pCondition->Info.Need_Confirm == 0x01){
		EnPacket_DefaultAckAKV(pData, pCondition, &defaultAKV_len);
		pData = pData + defaultAKV_len;
	}

#ifdef _Optional_AKV_EN_
	memcpy(pData, InZone_ACK_DataPointer.pStart, (InZone_ACK_DataPointer.pEnd - InZone_ACK_DataPointer.pStart));
	pData = pData + (InZone_ACK_DataPointer.pEnd - InZone_ACK_DataPointer.pStart);

	// --- qyn attr ---
	// attr
	*(pData++) = 0x00;
	// K_len
	*(pData++) = 0x03;
	// key
	*(pData++) = 'q';
	*(pData++) = 'y';
	*(pData++) = 'n';
	// V_len
	*(pData++) = 0x00;
	*(pData++) = 0x01;
	// value
	*(pData++) = _Max_Zone;
#endif

	// prepare to send packet
	Packet.length = pData - ((uint8_t *)&(Packet.Data[0]));
	
	// get the crc16
	pData         = (uint8_t *)&(Packet.Data[0]) + 2;	
	crc_resault   = Caculate(pData, (Packet.length-2));
	PacketHead->crc16 = HTONS(crc_resault);
	
	// send packet
	res = UDP_SendPacket((char *)&(Packet.Data[0]), Packet.length);
	
	if(res!=ERR_NONE){
		DEBUG("%s : ACK ...Error!\n",__func__);
		return APP_ERR_SEND;
	} else {
		DEBUG("%s : ACK ...OK\n",__func__);
		return APP_ERR_NONE;
	}
}


static void EnPacket_DefaultAckAKV(uint8_t *pdata, InZone_Report_Struct *pCondition, uint16_t *pLength)
{
	uint8_t 	*pdata_iner;
	uint16_t 	tmp;

	pdata_iner = pdata;

	// --- oN attr ---
	// attr
	*(pdata_iner++) = 0x00;
	// K_len
	*(pdata_iner++) = 0x02;
	// key
	*(pdata_iner++) = 'o';
	*(pdata_iner++) = 'N';
	// V_len
	*(pdata_iner++) = 0x00;
	*(pdata_iner++) = 0x01;
	// value
	*(pdata_iner++) = pCondition->Info.On_now;

	// --- p attr ---
	// attr
	*(pdata_iner++) = 0x00;
	// K_len
	*(pdata_iner++) = 0x01;
	// key
	*(pdata_iner++) = 'p';
	// V_len
	*(pdata_iner++) = 0x00;
	*(pdata_iner++) = 0x01;
	// value
	*(pdata_iner++) = pCondition->Info.Priority;

	// --- ts attr ---
	// attr
	*(pdata_iner++) = 0x00;
	// K_len
	*(pdata_iner++) = 0x02;
	// key
	*(pdata_iner++) = 't';	
	*(pdata_iner++) = 's';
	// V_len
	*(pdata_iner++) = 0x00;
	*(pdata_iner++) = 0x02;
	// value
	tmp = pCondition->Info.Cnt;
	*(pdata_iner++) = (uint8_t)((tmp>>8) & 0x00FF);
	*(pdata_iner++) = (uint8_t)((tmp) & 0x00FF);

	// --- i attr ---
	// attr
	*(pdata_iner++) = 0x00;
	// K_len
	*(pdata_iner++) = 0x01;
	// key
	*(pdata_iner++) = 'i';	
	// V_len
	*(pdata_iner++) = 0x00;
	*(pdata_iner++) = 0x02;
	// value
	tmp = pCondition->Info.Interval;
	*(pdata_iner++) = (uint8_t)((tmp>>8) & 0x00FF);
	*(pdata_iner++) = (uint8_t)((tmp) & 0x00FF);

	(*pLength) = (pdata_iner - pdata);
}




// =====================================================================================

static APP_Error OutZone_Report(CommandType *command)
{
	uint8_t 	*pdata;				// ponit to the data buff
	uint16_t 	tmp;
	uint8_t 	i;			
	uint8_t 	id_len;				// id length
	uint8_t 	id_tmp[_RID_LEN];	// buff the id
	uint8_t 	Need_Confirm;		// if the packet ack need to add the rule ack
	uint8_t		On_now;				// if the Report condition to be open right now
	uint8_t		Priority;			
	uint16_t	Cnt;
	uint16_t	Interval;
	uint32_t	Report_index;
	uint8_t		Shape;

	uint8_t		*pData_start;
	uint8_t		*pData_end;
	uint8_t		*pPacket_end;
	int			key_len;
	int			key_str;
	void        *pShape_Value;		// pointer to the memery of shape value
	TimeSet_Struct TimeSet_i;		// time set
	
	InZone_Report_Struct *pCondition;

	pdata = command->pData;

	// get the report id_len and id
	id_len = *(pdata++);
	DEBUG("%s : [id_len = %d]\n",__func__,id_len);
	assert(id_len <= _RID_LEN);		// we assert that the id_len < _RID_LEN
	memset(id_tmp,0,_RID_LEN);		// clear the id str
	for(i=0;i<id_len;i++){
		id_tmp[i] = *(pdata++);
	}
	DEBUG("%s : [ID : %s]\n",__func__,id_tmp);


	// if the packet ack need to add the rule ack 
	Need_Confirm = *(pdata++);
	DEBUG("%s : Need_Confirm : [%d]\n",__func__,Need_Confirm);

	// get the on_now value
	On_now = *(pdata++);
	DEBUG("%s : On_now : [%d]\n",__func__,On_now);

	// get the Priority value
	Priority = *(pdata++);
	DEBUG("%s : Priority : [%d]\n",__func__,Priority);

	// get the Cnt value
	tmp = *(pdata++);
	tmp = ((tmp<<8)&0xFF00) + *(pdata++);
	Cnt = tmp;
	DEBUG("%s : Cnt : [%d]\n",__func__,Cnt);

	// get the Interval value
	tmp = *(pdata++);
	tmp = ((tmp<<8)&0xFF00) + *(pdata++);
	Interval = tmp;
	DEBUG("%s : Interval : [%d]\n",__func__,Interval);

//==============================================================================
	// 解析可选可选参数，参数顺序不定，要特别注意~~
	OutZone_ACK_DataPointer.pStart = pdata;												// we need this for ack
	pPacket_end = command->pData + command->length; 	
	DEBUG("%s : pPacket End address : [ 0x%08X ]\n",__func__,(int)pPacket_end);
	pData_start = pdata;
	pData_end   = pdata;
	
	while(pData_start < pPacket_end){		// paser the data ,untill reach the end
		key_len = *(pData_start + 1) & 0x00FF;
		
		if(key_len==1) {
			key_str = *(pData_start + 2);
			if(key_str=='T'){
				T_AKV_Paser(pData_start, &TimeSet_i, &pData_end);
			} else {	
				DEBUG("%s : Key str : [ 0x%08X ]\n",__func__,key_str);
				DEBUG("%s : Paser the AKV...ERROR\n",__func__);
			}
		} else if(key_len==2) {
			key_str = *(pData_start + 2);
			key_str = (key_str<<8) + *(pData_start + 3);
			if(key_str==_ix_AKV){
				ix_AKV_Paser(pData_start, &Report_index, &pData_end);
			} else if(key_str==_rS_AKV) {
				rS_AKV_Paser(pData_start, Shape, &pShape_Value, &pData_end);
			} else if(key_str==_rT_AKV) {
				rT_AKV_Paser(pData_start, &Shape, &pData_end);
			} else {
				DEBUG("%s : Paser the AKV...ERROR\n",__func__);	
				DEBUG("%s : Key str : [ 0x%08X ]\n",__func__,key_str);
			}
		} else {
			DEBUG("%s : Paser the AKV...ERROR\n",__func__);		
			DEBUG("%s : pData End address : [ 0x%08X ]\n",__func__,(int)pData_end);
		}
		pData_start = pData_end;
	}
//==============================================================================	
		
	DEBUG("%s : AKV Paser over... \n",__func__);
	if(ReportCondition_Search(Report_index, pOutZoneCondition_List, &pCondition) != 1){		// we can't find the report condition
		pCondition = malloc(sizeof(InZone_Report_Struct));									// malloc ram for the report condition
		memset(pCondition, 0, sizeof(InZone_Report_Struct));								// clear the rule
		ReportCondition_Add(&(pCondition->list),pOutZoneCondition_List);
	} else {
		free(pCondition->pShape_Value);
	}

	// ---- save the value ----
	// save id
		
	DEBUG("%s : pCondition address : [ 0x%08X ]\n",__func__,(int)pCondition);
	DEBUG("%s : Save the value... \n",__func__);
	memcpy(pCondition->Info.RID, id_tmp, _RID_LEN);
	pCondition->Info.R_len 		  = id_len;
	pCondition->Info.Need_Confirm = Need_Confirm;
	pCondition->Info.On_now 	  = On_now;
	pCondition->Info.Priority 	  = Priority;
	pCondition->Info.Cnt 		  = Cnt;
	pCondition->Info.Interval 	  = Interval;
	pCondition->Info.action 	  = command->Action;

	pCondition->index = Report_index;
	pCondition->Shape = Shape;

	pCondition->pShape_Value = pShape_Value;
	memcpy((char *)(&(pCondition->TimeSet)), (char *)&TimeSet_i, sizeof(TimeSet_Struct));
	
	OutZone_ACK_DataPointer.pEnd = 0;													// we need this for ack
	OutZone_ACK_DataPointer.pCondition = pCondition;

	OutZone_ACK_DataPointer.ACK = 0x01;													// default, we send OK
	

	DEBUG("%s : parser over... \n",__func__);
	return APP_ERR_NONE;
}


static APP_Error OutZone_Report_ACK(CommandType *command)
{

	GSM_Error		res;
	Packet_Struct   Packet;
	uint16_t		crc_resault;
	TLP_Head_Struct	*PacketHead;
	uint8_t			*pData;

	uint16_t 		defaultAKV_len;
	InZone_Report_Struct *pCondition;

	PacketHead = (TLP_Head_Struct *)&(Packet.Data[0]);
	pData      = (uint8_t *)&(Packet.Data[0]) + sizeof(TLP_Head_Struct);

	pCondition = (InZone_Report_Struct *)OutZone_ACK_DataPointer.pCondition;

	// no need to change
	PacketHead->version = _VERSION;
	PacketHead->length  = sizeof(TLP_Head_Struct);
	PacketHead->type    = _TransmitCtrl;			// device to server
	PacketHead->medium  = _TransmitType;			// UDP
	PacketHead->encrypt = _Encrypt;					// encrypt none
	PacketHead->reserve = _Reserve;			
	
	PacketHead->DEV_IDH = HTONL(_DEV_IDH);
	PacketHead->DEV_IDL = HTONL(_DEV_IDL);
	
	//need to change
	PacketHead->timestamp = 0x00;					// need to fix
	PacketHead->SEQ_num = command->plowLevelInfo->SEQ_num;

	// send the ack
	*(pData++) = REPORT_SET;
	*(pData++) = (uint8_t)((_OUT_ZONE>>8) & 0x00FF);
	*(pData++) = (uint8_t)(_OUT_ZONE & 0x00FF);						// action

	*(pData++) = pCondition->Info.R_len;							// R_len;

	memcpy(pData, pCondition->Info.RID, pCondition->Info.R_len);	// RID
	pData = pData + pCondition->Info.R_len;

	*(pData++) = OutZone_ACK_DataPointer.ACK;								// ACK

	if(pCondition->Info.Need_Confirm == 0x01){
		EnPacket_DefaultAckAKV(pData, pCondition, &defaultAKV_len);
		pData = pData + defaultAKV_len;
	}

#ifdef _Optional_AKV_EN_
	memcpy(pData, OutZone_ACK_DataPointer.pStart, (OutZone_ACK_DataPointer.pEnd - OutZone_ACK_DataPointer.pStart));
	pData = pData + (OutZone_ACK_DataPointer.pEnd - OutZone_ACK_DataPointer.pStart);

	// --- qyn attr ---
	// attr
	*(pData++) = 0x00;
	// K_len
	*(pData++) = 0x03;
	// key
	*(pData++) = 'q';
	*(pData++) = 'y';
	*(pData++) = 'n';
	// V_len
	*(pData++) = 0x00;
	*(pData++) = 0x01;
	// value
	*(pData++) = _Max_Zone;
#endif

	// prepare to send packet
	Packet.length = pData - ((uint8_t *)&(Packet.Data[0]));
	
	// get the crc16
	pData         = (uint8_t *)&(Packet.Data[0]) + 2;	
	crc_resault   = Caculate(pData, (Packet.length-2));
	PacketHead->crc16 = HTONS(crc_resault);
	
	// send packet
	res = UDP_SendPacket((char *)&(Packet.Data[0]), Packet.length);
	
	if(res!=ERR_NONE){
		DEBUG("%s : ACK ...Error!\n",__func__);
		return APP_ERR_SEND;
	} else {
		DEBUG("%s : ACK ...OK\n",__func__);
		return APP_ERR_NONE;
	}
}



// ===================================================================
void ShowCondition(struct list_head *phead)
{
	struct list_head *plist;
	InZone_Report_Struct *pCondition;
	
	list_for_each(plist, phead) {
		pCondition = list_entry(plist, InZone_Report_Struct, list);
		
		Condition_Printf(pCondition);
		fprintf(stderr,"=========================\n");
	}
}

static void Condition_Printf(InZone_Report_Struct *pCondition)
{
	int i;
	
	fprintf(stderr,"Condition RID   : %s\n", pCondition->Info.RID);
	fprintf(stderr,"Need Confirm    : %d\n", pCondition->Info.Need_Confirm);
	fprintf(stderr,"On Now          : %d\n", pCondition->Info.On_now);
	fprintf(stderr,"Priority        : %d\n", pCondition->Info.Priority);
	fprintf(stderr,"Cnt             : %d\n", pCondition->Info.Cnt);
	fprintf(stderr,"Interval        : %d\n", pCondition->Info.Interval);
	fprintf(stderr,"Condition index : %d\n", pCondition->index);
	
	fprintf(stderr,"Zone Shape      : %02X (0x01 : circle; 0x02 : rectangle;  0x03 : polygon)\n", pCondition->Shape);
	
	if(pCondition->Shape == _Circle){
		CirclePrintf((Circle_Struct *)pCondition->pShape_Value);
	} else if(pCondition->Shape == _Rectangle){
		RectanglePrintf((Rectangle_Struct *)pCondition->pShape_Value);
	} else if(pCondition->Shape == _Polygon){
		PolygonPrintf((Polygon_Struct *)pCondition->pShape_Value);
	}
	
	fprintf(stderr,"The time Set:\n");
	for(i=0;i<pCondition->TimeSet.TimeSet_Count;i++){
		fprintf(stderr,"  *Time node %d :\n",i);
		BEtimePrintf(&(pCondition->TimeSet.BE_Time[i]));
	}
}

static void PointPrintf(Point_Struct *pPoint)
{
	fprintf(stderr,"	| Status    : %d\n", pPoint->location_Status);
	fprintf(stderr,"	| Latitude  : %d\n", pPoint->Latitude);
	fprintf(stderr,"	| Longitude : %d\n", pPoint->Longitude);
	fprintf(stderr,"	| Alitude   : %d\n", pPoint->Alitude);
}

static void CirclePrintf(Circle_Struct *pCircle)
{
	fprintf(stderr,"  this is a Circle\n");
	fprintf(stderr,"  *The Center :\n");
	PointPrintf(&(pCircle->Center));
	fprintf(stderr,"  *The Radius :\n");
	fprintf(stderr,"	| %d M\n", pCircle->Radius);
}

static void RectanglePrintf(Rectangle_Struct *pRectangle)
{
	fprintf(stderr,"  this is a Rectangle\n");
	fprintf(stderr,"  *The Top left corner :\n");
	PointPrintf(&(pRectangle->Top_left_corner));
	fprintf(stderr,"  *Bottom right corner :\n");
	PointPrintf(&(pRectangle->Bottom_right_corner));
}

static void PolygonPrintf(Polygon_Struct *pPolygon)
{
	int i;
	fprintf(stderr,"  this is a Polygon\n");
	
	for(i=0;i<pPolygon->Corner_count;i++){
		fprintf(stderr,"  *The corner %d :\n",i);
		PointPrintf(&(pPolygon->Corner[i]));
	}
}

void BEtimePrintf(TimeSlot_Struct *pTimeSlot)
{
	fprintf(stderr,"	| Btime     : %02d:%02d:%02d\n", pTimeSlot->Btime.hour,pTimeSlot->Btime.minute,pTimeSlot->Btime.second);
	fprintf(stderr,"	| Etime     : %02d:%02d:%02d\n", pTimeSlot->Etime.hour,pTimeSlot->Etime.minute,pTimeSlot->Etime.second);
}





//--------------------------------------------------------------------------------------------
// add by shmily -- 2012.09.26
// tips: 可选参数是无序的，分开来解析，否则会出错
//-------------------------------------------------------------------------------------------

static void ix_AKV_Paser(uint8_t * pData_start, uint32_t *pIndex, uint8_t **pData_end)
{
	uint8_t 	*pdata;
	uint32_t	Report_index;
	
	// for AKV parser, no need to save
	uint8_t		Attr;
	uint8_t		K_len;
	uint16_t	V_len;
	uint8_t		*pKey;
	uint8_t		*pValue;
	
	DEBUG("%s : Paser the ix attr...\n",__func__);
	
	pdata = pData_start;
	R_SimpleAKV_Parser(pdata, &Attr, &K_len, &V_len, &pKey, &pValue);
	
	assert(Attr  == 0);
	assert(K_len == 2);
	assert(V_len == 4);
	
	Report_index = 0;
	Report_index = (*pValue++) & 0x000000FF;
	Report_index = (Report_index<<8) + ((*pValue++) & 0x00FF);
	Report_index = (Report_index<<8) + ((*pValue++) & 0x000000FF);
	Report_index = (Report_index<<8) + ((*pValue++) & 0x000000FF);
	DEBUG("%s : Report_index : [%d]\n",__func__,Report_index);	
	
	(*pIndex) = Report_index;
	(*pData_end) = pValue;
}


static void rT_AKV_Paser(uint8_t * pData_start, uint8_t *pShape, uint8_t **pData_end)
{
	uint8_t 	*pdata;
	uint8_t		Shape;
	
	// for AKV parser, no need to save
	uint8_t		Attr;
	uint8_t		K_len;
	uint16_t	V_len;
	uint8_t		*pKey;
	uint8_t		*pValue;

	DEBUG("%s : Paser the rT attr...\n",__func__);
	
	pdata = pData_start;
	R_SimpleAKV_Parser(pdata, &Attr, &K_len, &V_len, &pKey, &pValue);
	
	assert(Attr  == 0);
	assert(K_len == 2);
	assert(V_len == 1);
	Shape = (*pValue++);
	DEBUG("%s : Shape : [%d]\n",__func__,Shape);
	
	(*pShape) = Shape;
	(*pData_end) = pValue;
}

static void rS_AKV_Paser(uint8_t * pData_start, uint8_t Shape, void **pSave_node, uint8_t **pData_end)
{
	DEBUG("%s : Paser the rS attr...\n",__func__);
	ShapeValue_malloc(Shape, pData_start, pData_end, pSave_node);	
}

static void T_AKV_Paser(uint8_t * pData_start, TimeSet_Struct *pTimeSet, uint8_t **pData_end)
{
	DEBUG("%s : Paser the T attr...\n",__func__);
	BeTimeAKV_Parser(pData_start, pData_end, pTimeSet);
}

