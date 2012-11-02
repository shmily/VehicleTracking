/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-10-11 11:32
#      Filename : ReportCondition_Speed.c
#   Description : not thing...
#
=============================================================================*/

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "ReportConditionList.h"
#include "ReportCondition.h"
#include "UDP_Lib.h"
#include "crc16.h"

#include "ReportCondition_Speed.h"


#define		_ix_AKV					((('i'<<8)|('x')) & 0xFFFF)

struct list_head 		*pSACondition_List;
ACK_DataPointer_Struct	SA_ACK_DataPointer;


static void TDSA_EnPacket_DefaultAckAKV(uint8_t *pdata, SpeedAnomaly_Report *pCondition, uint16_t *pLength);
static void Condition_Printf(SpeedAnomaly_Report *pCondition);

static void ix_AKV_Paser(uint8_t * pData_start, uint32_t *pIndex, uint8_t **pData_end);
static void s_AKV_Paser(uint8_t * pData_start, uint8_t *pSpeed, uint8_t **pData_end);
static void T_AKV_Paser(uint8_t * pData_start, TimeSet_Struct *pTimeSet, uint8_t **pData_end);
static void I_AKV_Paser(uint8_t * pData_start, uint16_t *pInterval, uint8_t **pData_end);



// time depend speed anomaly 
void TDSA_Condition_Initial(struct list_head *phead)
{
	INIT_LIST_HEAD(phead);
	pSACondition_List = phead;
}


APP_Error TimeDepend_SpeedAnomaly_Report(CommandType *command)
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
	uint32_t	Report_index = 0;
	uint8_t		speed;

	SpeedAnomaly_Report *pCondition;
	
	uint8_t		*pData_start;
	uint8_t		*pData_end;
	uint8_t		*pPacket_end;
	int			key_len;
	int			key_str;
	uint16_t	Interval_t;
	TimeSet_Struct TimeSet_i;		// time set	
	

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
	
	// ==========================================================================
	// 解析可选可选参数，参数顺序不定，要特别注意~~
	
	pPacket_end = command->pData + command->length;
	DEBUG("%s : pPacket End address : [ 0x%08X ]\n",__func__,(int)pPacket_end);
	pData_start = pdata;
	pData_end   = pdata;
	
	while(pData_start < pPacket_end){
		key_len = *(pData_start + 1) & 0x00FF;
		
		if(key_len==1){
			key_str = *(pData_start + 2);
			if(key_str=='s') {
				s_AKV_Paser(pData_start, &speed, &pData_end);
			} else if(key_str=='T'){
				T_AKV_Paser(pData_start, &TimeSet_i, &pData_end);
			} else if(key_str=='l'){
				I_AKV_Paser(pData_start, &Interval_t, &pData_end);
			} else {
				DEBUG("%s : Key str : [ 0x%08X ]\n",__func__,key_str);
				DEBUG("%s : Paser the AKV...ERROR\n",__func__);			
			}
		} else if(key_len==2){
			key_str = *(pData_start + 2);
			key_str = (key_str<<8) + *(pData_start + 3);
			
			if(key_str==_ix_AKV){
				ix_AKV_Paser(pData_start, &Report_index, &pData_end);
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
	
//==========================================================================
	

	if(TDSA_ReportCondition_Search(Report_index, pSACondition_List, &pCondition) != 1){		// we can't find the report condition
		pCondition = malloc(sizeof(SpeedAnomaly_Report));									// malloc ram for the report condition
		memset(pCondition, 0, sizeof(SpeedAnomaly_Report));									// clear the rule
		ReportCondition_Add(&(pCondition->list),pSACondition_List);
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
	pCondition->speed = speed;
	pCondition->Interval_t = Interval_t;
	
	memcpy((char *)(&(pCondition->TimeSet)), (char *)&TimeSet_i, sizeof(TimeSet_Struct));
	
	SA_ACK_DataPointer.pCondition = pCondition;
	SA_ACK_DataPointer.ACK = 0x01;												// default, we send OK

	return APP_ERR_NONE;
}


APP_Error TDSA_Report_ACK(CommandType *command)
{

	GSM_Error		res;
	Packet_Struct   Packet;
	uint16_t		crc_resault;
	TLP_Head_Struct	*PacketHead;
	uint8_t			*pData;

	uint16_t 		defaultAKV_len;
	SpeedAnomaly_Report *pCondition;

	PacketHead = (TLP_Head_Struct *)&(Packet.Data[0]);
	pData      = (uint8_t *)&(Packet.Data[0]) + sizeof(TLP_Head_Struct);

	pCondition = (SpeedAnomaly_Report *)SA_ACK_DataPointer.pCondition;

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
	*(pData++) = (uint8_t)((_TD_SPEED>>8) & 0x00FF);
	*(pData++) = (uint8_t)(_TD_SPEED & 0x00FF);						// action

	*(pData++) = pCondition->Info.R_len;							// R_len;

	memcpy(pData, pCondition->Info.RID, pCondition->Info.R_len);	// RID
	pData = pData + pCondition->Info.R_len;

	*(pData++) = SA_ACK_DataPointer.ACK;						// ACK

	if(pCondition->Info.Need_Confirm == 0x01){
		TDSA_EnPacket_DefaultAckAKV(pData, pCondition, &defaultAKV_len);
		pData = pData + defaultAKV_len;
	}


#ifdef _Optional_AKV_EN_

	// --- i attr ---
	// attr
	*(pData++) = 0x00;
	// K_len
	*(pData++) = 0x01;
	// key
	*(pData++) = 'i';
	// V_len
	*(pData++) = 0x00;
	*(pData++) = 0x04;
	// value
	*(pData++) = (uint8_t)((pCondition->index)>>24);
	*(pData++) = (uint8_t)((pCondition->index)>>16);
	*(pData++) = (uint8_t)((pCondition->index)>>8);
	*(pData++) = (uint8_t)((pCondition->index));

	memcpy(pData, SA_ACK_DataPointer.pStart, (SA_ACK_DataPointer.pEnd - SA_ACK_DataPointer.pStart));
	pData = pData + (SA_ACK_DataPointer.pEnd - SA_ACK_DataPointer.pStart);

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


static void TDSA_EnPacket_DefaultAckAKV(uint8_t *pdata, SpeedAnomaly_Report *pCondition, uint16_t *pLength)
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


// ===================================================================
void ShowSpeedCondition(struct list_head *phead)
{
	struct list_head *plist;
	SpeedAnomaly_Report *pCondition;
	
	list_for_each(plist, phead) {
		pCondition = list_entry(plist, SpeedAnomaly_Report, list);
		
		Condition_Printf(pCondition);
		fprintf(stderr,"=========================\n");
	}
}

static void Condition_Printf(SpeedAnomaly_Report *pCondition)
{
	int i;
	
	fprintf(stderr,"Condition RID   : %s\n", pCondition->Info.RID);
	fprintf(stderr,"Need Confirm    : %d\n", pCondition->Info.Need_Confirm);
	fprintf(stderr,"On Now          : %d\n", pCondition->Info.On_now);
	fprintf(stderr,"Priority        : %d\n", pCondition->Info.Priority);
	fprintf(stderr,"Cnt             : %d\n", pCondition->Info.Cnt);
	fprintf(stderr,"Interval        : %d\n", pCondition->Info.Interval);
	fprintf(stderr,"Condition index : %d\n", pCondition->index);
	fprintf(stderr,"speed           : %d\n", pCondition->speed);
	fprintf(stderr,"Interval_t      : %d\n", pCondition->Interval_t);
	
	fprintf(stderr,"The time Set:\n");
	for(i=0;i<pCondition->TimeSet.TimeSet_Count;i++){
		fprintf(stderr,"  *Time node %d :\n",i);
		BEtimePrintf(&(pCondition->TimeSet.BE_Time[i]));
	}
}

//--------------------------------------------------------------------------------------------
// add by shmily -- 2012.10.11
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
	
	// paser the "ix" attr
	DEBUG("%s : Paser the ix attr...\n",__func__);
	
	pdata = pData_start;
	R_SimpleAKV_Parser(pdata, &Attr, &K_len, &V_len, &pKey, &pValue);
	
	assert(Attr == 0);
	assert(K_len == 2);
	assert(V_len == 4);
	
	Report_index = 0x00000000;
	Report_index = (*pValue++) & 0x000000FF;
	Report_index = (Report_index<<8) + ((*pValue++) & 0x00FF);
	Report_index = (Report_index<<8) + ((*pValue++) & 0x000000FF);
	Report_index = (Report_index<<8) + ((*pValue++) & 0x000000FF);
	
	DEBUG("%s : Report_index : [%d]\n",__func__,Report_index);
	
	(*pIndex) = Report_index;
	(*pData_end) = pValue;
}

// paser the "s" attr
static void s_AKV_Paser(uint8_t * pData_start, uint8_t *pSpeed, uint8_t **pData_end)
{
	uint8_t 	*pdata;
	uint8_t		speed;
	
	// for AKV parser, no need to save
	uint8_t		Attr;
	uint8_t		K_len;
	uint16_t	V_len;
	uint8_t		*pKey;
	uint8_t		*pValue;	
	
	pdata = pData_start;		
	DEBUG("%s : Paser the rT attr...\n",__func__);
	R_SimpleAKV_Parser(pdata, &Attr, &K_len, &V_len, &pKey, &pValue);	
	assert(Attr == 0);
	assert(K_len == 1);
	assert(V_len == 1);
	speed = (*pValue++);
	
	(*pSpeed) = speed;
	(*pData_end) = pValue;	
}

static void T_AKV_Paser(uint8_t * pData_start, TimeSet_Struct *pTimeSet, uint8_t **pData_end)
{
	DEBUG("%s : Paser the T attr...\n",__func__);
	BeTimeAKV_Parser(pData_start, pData_end, pTimeSet);
}


static void I_AKV_Paser(uint8_t * pData_start, uint16_t *pInterval, uint8_t **pData_end)
{
	uint8_t 	*pdata;
	uint16_t	tmp;
	
	// for AKV parser, no need to save
	uint8_t		Attr;
	uint8_t		K_len;
	uint16_t	V_len;
	uint8_t		*pKey;
	uint8_t		*pValue;		
	
	// paser the "I" AKV
	pdata = pData_start;
	R_SimpleAKV_Parser(pdata, &Attr, &K_len, &V_len, &pKey, &pValue);
	assert(Attr == 0);
	assert(K_len == 1);
	assert(V_len == 2);
	tmp = *(pValue++);
	tmp = ((tmp<<8)&0xFF00) + *(pValue++);
	
	(*pInterval) = tmp;
	(*pData_end) = pValue;
}
